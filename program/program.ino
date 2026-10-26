/*
 * Porte-cles NFC - 4 profils (bleu / rouge / vert / violet)
 * MCU  : ATtiny85V-10SU (8 broches)      NFC : ST25DV04KC-IER6S3 (I2C + antenne PCB)
 * LED  : Kingbright APTF1616LSEEZGKQBKC (RGB, anode commune)
 * Pile : CR2032 (3 V)
 *
 * Brochage ATtiny85 (numero de patte du boitier SOIC-8)
 *   1 PB5 /RESET  -> en-tete ISP uniquement (+ pull-up 10k)
 *   2 PB3         -> LED verte  (cathode, via R)
 *   3 PB4         -> LED bleue  (cathode, via R)
 *   4 GND
 *   5 PB0         -> SDA  (ST25DV patte 5)  + pull-up 10k   [ISP: MOSI]
 *   6 PB1         -> LED rouge  (cathode, via R)            [ISP: MISO]
 *   7 PB2         -> SCL  (ST25DV patte 6)  + pull-up 10k + BOUTON vers GND  [ISP: SCK]
 *   8 VCC
 *
 * Fonctionnement
 *   OFF  : RF du ST25DV coupee (RF_SLEEP=1, le tag ne repond plus du tout), MCU en power-down.
 *   Appui court en OFF       -> ON, profil memorise, LED allumee 1,5 s
 *   Appui court en ON        -> profil suivant (bleu>rouge>vert>violet>bleu)
 *   Appui long (>=1 s) en ON -> OFF (flash blanc)
 *   En ON : petit flash de la couleur du profil toutes les ~4 s
 *   Erreur I2C : 4 flashs jaunes (rouge+vert)
 *
 * Reglages IDE (Arduino IDE 2.x + ATTinyCore) : voir README.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <stdint.h>

#ifndef F_CPU
#define F_CPU 1000000UL          /* ATtiny85 sorti d'usine : 8 MHz / 8 */
#endif

/* ===================== A PERSONNALISER : les 4 URL ===================== */
/* Ecrire l'URL complete. "https://" et "http://" sont compactes
   automatiquement (code NDEF 0x04 / 0x03). Longueur max : 116 caracteres.  */
static const char URL_0[] PROGMEM = "https://example.com/bleu";    /* profil 0 : BLEU   */
static const char URL_1[] PROGMEM = "https://example.com/rouge";   /* profil 1 : ROUGE  */
static const char URL_2[] PROGMEM = "https://example.com/vert";    /* profil 2 : VERT   */
static const char URL_3[] PROGMEM = "https://example.com/violet";  /* profil 3 : VIOLET */
/* ======================================================================= */

static const char *const URLS[4] PROGMEM = { URL_0, URL_1, URL_2, URL_3 };

/* ---- broches ---- */
#define K_SDA   PB0
#define K_SCL   PB2          /* partagee avec le bouton */
#define K_LR    PB1
#define K_LG    PB3
#define K_LB    PB4
#define LED_ALL (_BV(K_LR) | _BV(K_LG) | _BV(K_LB))

/* couleur de chaque profil (broches LED a mettre a 0 V) */
static const uint8_t COLOR[4] PROGMEM = {
  _BV(K_LB),                 /* 0 bleu   */
  _BV(K_LR),                 /* 1 rouge  */
  _BV(K_LG),                 /* 2 vert   */
  _BV(K_LR) | _BV(K_LB)      /* 3 violet */
};

#define LONG_MS        1000  /* duree d'appui long                     */
#define SHOW_MS        1500  /* LED fixe apres un changement           */
#define EE_PROFILE     ((uint8_t *)0)

/* ---- ST25DV04KC ---- */
#define ST_DEV_W        0xA6          /* (0x53 << 1) : memoire utilisateur + registres dynamiques */
#define ST_DEV_R        0xA7
#define ST_RF_MNGT_DYN  0x2003        /* bit0 RF_DISABLE, bit1 RF_SLEEP, bit2 RF_OFF (RO) */

/* ---- etat ---- */
static uint8_t           g_on;        /* 1 = NFC actif */
static uint8_t           g_prof;      /* 0..3 */
static volatile uint8_t  g_wdt;       /* reveil watchdog */
static uint8_t           i2c_fail;
#define NDEF_BUF 128
static uint8_t           nbuf[NDEF_BUF];

/* ---- ISR ---- */
ISR(PCINT0_vect) { /* reveil par le bouton : rien a faire */ }
ISR(WDT_vect)    { g_wdt = 1; }

/* ---- utilitaires ---- */
static void wait_ms(uint16_t ms) { while (ms--) _delay_ms(1); }

/* LED a anode commune : broche en sortie a 0 = allumee, broche en entree (Hi-Z) = eteinte */
static void led(uint8_t mask) {
  DDRB = (uint8_t)((DDRB & (uint8_t)~LED_ALL) | (mask & LED_ALL));
}
static uint8_t prof_color(void) { return pgm_read_byte(&COLOR[g_prof]); }

/* bouton = SCL tire a 0 (pull-up externe 10k). Renvoie 1 si appuye. */
static uint8_t btn(void) {
  DDRB &= (uint8_t)~_BV(K_SCL);
  _delay_us(30);
  return (PINB & _BV(K_SCL)) ? 0 : 1;
}

/* ===================== I2C logiciel (open-drain, ~50 kHz) ===================== */
#define I2C_T() _delay_us(5)
static inline void sda_lo(void)  { DDRB |=  _BV(K_SDA); }
static inline void sda_rel(void) { DDRB &= (uint8_t)~_BV(K_SDA); }
static inline void scl_lo(void)  { DDRB |=  _BV(K_SCL); }
static void scl_rel(void) {
  DDRB &= (uint8_t)~_BV(K_SCL);
  uint8_t t = 100;                       /* si SCL reste a 0 (bouton appuye) -> erreur */
  while (!(PINB & _BV(K_SCL))) {
    if (--t == 0) { i2c_fail = 1; return; }
    _delay_us(2);
  }
}
static void i2c_start(void) { sda_rel(); scl_rel(); I2C_T(); sda_lo(); I2C_T(); scl_lo(); I2C_T(); }
static void i2c_stop(void)  { sda_lo(); I2C_T(); scl_rel(); I2C_T(); sda_rel(); I2C_T(); }

static uint8_t i2c_wr(uint8_t b) {       /* renvoie 1 si ACK */
  for (uint8_t i = 0; i < 8; i++) {
    if (b & 0x80) sda_rel(); else sda_lo();
    I2C_T(); scl_rel(); I2C_T(); scl_lo();
    b <<= 1;
  }
  sda_rel(); I2C_T(); scl_rel(); I2C_T();
  uint8_t ack = (PINB & _BV(K_SDA)) ? 0 : 1;
  scl_lo();
  return ack;
}
static uint8_t i2c_rd(uint8_t ack) {
  uint8_t b = 0;
  sda_rel();
  for (uint8_t i = 0; i < 8; i++) {
    I2C_T(); scl_rel(); I2C_T();
    b = (uint8_t)((b << 1) | ((PINB & _BV(K_SDA)) ? 1 : 0));
    scl_lo();
  }
  if (ack) sda_lo(); else sda_rel();
  I2C_T(); scl_rel(); I2C_T(); scl_lo();
  sda_rel();
  return b;
}

/* ===================== acces ST25DV04KC ===================== */
/* attend la fin du cycle d'ecriture EEPROM (ACK polling, ~5 ms typ.) */
static uint8_t st_ack_poll(void) {
  for (uint8_t i = 0; i < 40; i++) {
    i2c_start();
    uint8_t a = i2c_wr(ST_DEV_W);
    i2c_stop();
    if (a) return 1;
    wait_ms(1);
  }
  return 0;
}
static uint8_t st_write(uint16_t addr, const uint8_t *d, uint8_t n) {
  i2c_fail = 0;
  i2c_start();
  uint8_t ok = i2c_wr(ST_DEV_W) && i2c_wr((uint8_t)(addr >> 8)) && i2c_wr((uint8_t)(addr & 0xFF));
  for (uint8_t i = 0; ok && i < n; i++) ok = i2c_wr(d[i]);
  i2c_stop();
  if (!ok || i2c_fail) return 0;
  return st_ack_poll();
}
static uint8_t st_read(uint16_t addr, uint8_t *d, uint8_t n) {
  i2c_fail = 0;
  i2c_start();
  uint8_t ok = i2c_wr(ST_DEV_W) && i2c_wr((uint8_t)(addr >> 8)) && i2c_wr((uint8_t)(addr & 0xFF));
  if (ok) { i2c_start(); ok = i2c_wr(ST_DEV_R); }
  for (uint8_t i = 0; ok && i < n; i++) d[i] = i2c_rd((uint8_t)(i < (uint8_t)(n - 1)));
  i2c_stop();
  return (ok && !i2c_fail) ? 1 : 0;
}

/* RF_SLEEP = 1 : le ST25DV ne repond plus a aucune commande RF.  0 : RF normale.
   Verifie par relecture. */
static uint8_t rf_set(uint8_t sleep) {
  uint8_t v = sleep ? 0x02 : 0x00, r = 0xFF;
  for (uint8_t i = 0; i < 20; i++) {
    if (st_write(ST_RF_MNGT_DYN, &v, 1) &&
        st_read(ST_RF_MNGT_DYN, &r, 1) && ((r & 0x03) == v)) return 1;
    wait_ms(5);                           /* RF occupee : on reessaie */
  }
  return 0;
}

/* ===================== message NDEF (Type 5 : CC + TLV) ===================== */
static uint8_t ndef_build(uint8_t p) {
  const char *u = (const char *)pgm_read_word(&URLS[p]);
  uint8_t code = 0x00, skip = 0;
  if (!strncmp_P("https://", u, 8))      { code = 0x04; skip = 8; }
  else if (!strncmp_P("http://", u, 7))  { code = 0x03; skip = 7; }
  size_t len = strlen_P(u);
  if (len <= skip || len > 116) return 0;          /* URL vide ou trop longue */
  uint8_t rem = (uint8_t)(len - skip), n = 0;

  nbuf[n++] = 0xE1; nbuf[n++] = 0x40; nbuf[n++] = 0x40; nbuf[n++] = 0x00;  /* Capability Container */
  nbuf[n++] = 0x03;                                /* TLV NDEF Message            */
  nbuf[n++] = (uint8_t)(5 + rem);                  /* longueur de l'enregistrement */
  nbuf[n++] = 0xD1;                                /* MB ME SR, TNF=1 (well-known) */
  nbuf[n++] = 0x01;                                /* longueur du type             */
  nbuf[n++] = (uint8_t)(1 + rem);                  /* longueur payload             */
  nbuf[n++] = 0x55;                                /* type 'U' (URI)               */
  nbuf[n++] = code;                                /* prefixe URI                  */
  memcpy_P(&nbuf[n], u + skip, rem); n += rem;
  nbuf[n++] = 0xFE;                                /* TLV terminateur              */
  return n;
}

/* ecrit seulement les blocs de 16 octets qui different (economise l'EEPROM) */
static uint8_t ndef_sync(uint8_t p) {
  uint8_t n = ndef_build(p);
  if (!n) return 0;
  uint8_t tmp[16];
  for (uint8_t off = 0; off < n; off = (uint8_t)(off + 16)) {
    uint8_t len = (uint8_t)(n - off);
    if (len > 16) len = 16;
    if (!st_read(off, tmp, len)) return 0;
    if (memcmp(tmp, &nbuf[off], len) != 0) {
      if (!st_write(off, &nbuf[off], len)) return 0;
      if (!st_read(off, tmp, len) || memcmp(tmp, &nbuf[off], len) != 0) return 0;
    }
  }
  return 1;
}

/* RF coupee pendant l'ecriture, activee seulement une fois le tag complet et verifie */
static uint8_t nfc_enable(void) {
  if (!rf_set(1)) return 0;
  if (!ndef_sync(g_prof)) return 0;
  return rf_set(0);
}
static uint8_t nfc_disable(void) { return rf_set(1); }

/* ===================== watchdog (reveil 4 s, mode interruption seul) ===================== */
static void wdt_start(void) {
  cli();
  wdt_reset();
  MCUSR &= (uint8_t)~_BV(WDRF);
  WDTCR |= _BV(WDCE) | _BV(WDE);
  WDTCR = _BV(WDIE) | _BV(WDP3);               /* 4,0 s */
  sei();
}
static void wdt_stop(void) {
  cli();
  wdt_reset();
  MCUSR &= (uint8_t)~_BV(WDRF);
  WDTCR |= _BV(WDCE) | _BV(WDE);
  WDTCR = 0;
  sei();
  g_wdt = 0;
}

/* ===================== retours visuels ===================== */
static void fail(void) {                        /* 4 flashs jaunes */
  for (uint8_t i = 0; i < 4; i++) { led(_BV(K_LR) | _BV(K_LG)); wait_ms(80); led(0); wait_ms(80); }
}
static void heartbeat(void) { led(prof_color()); wait_ms(25); led(0); }
/* LED fixe ~1,5 s ; sort tout de suite si on rappuie (renvoie 1) */
static uint8_t show_profile(void) {
  led(prof_color());
  for (uint16_t i = 0; i < SHOW_MS / 10; i++) {
    wait_ms(10);
    if (btn()) { led(0); return 1; }
  }
  led(0);
  return 0;
}

/* ===================== gestion du bouton ===================== */
static void handle_press(void) {
  wait_ms(30);
  if (!btn()) return;                           /* parasite / rebond */
  uint16_t held = 0;
  while (btn() && held < LONG_MS) { wait_ms(10); held += 10; }
  uint8_t is_long = (held >= LONG_MS);
  if (is_long && g_on) { led(LED_ALL); wait_ms(150); led(0); }   /* flash blanc = extinction */
  while (btn()) wait_ms(10);                    /* attendre le relachement (SCL doit etre libre pour l'I2C) */
  wait_ms(30);

  if (!g_on) {                                  /* OFF -> ON */
    if (nfc_enable()) { g_on = 1; wdt_start(); show_profile(); }
    else              { rf_set(1); fail(); }
  } else if (is_long) {                         /* ON -> OFF */
    uint8_t ok = 0;
    for (uint8_t i = 0; i < 3 && !ok; i++) ok = nfc_disable();
    if (ok) { g_on = 0; wdt_stop(); }
    else    { fail(); }                         /* reste ON : on le signale */
  } else {                                      /* ON : profil suivant */
    g_prof = (uint8_t)((g_prof + 1) & 3);
    eeprom_update_byte(EE_PROFILE, g_prof);
    if (nfc_enable()) { show_profile(); }
    else              { g_on = 0; wdt_stop(); rf_set(1); fail(); }
  }
}

/* ===================== mise en veille ===================== */
static void sleep_until_event(void) {
  led(0);
  DDRB &= (uint8_t)~(_BV(K_SDA) | _BV(K_SCL));  /* bus libere */
  GIFR  = _BV(PCIF);
  PCMSK = _BV(PCINT2);                          /* bouton sur PB2 */
  GIMSK |= _BV(PCIE);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
}

/* ===================== Arduino setup / loop ===================== */
void setup(void) {
  PORTB = 0x00;                                 /* aucune pull-up interne : le latch reste a 0 */
  DDRB  = 0x00;
  ADCSRA = 0;                                   /* ADC off */
  ACSR   = _BV(ACD);                            /* comparateur analogique off */
  TIMSK  = 0;                                   /* pas d'IT timer */
  PRR    = _BV(PRTIM1) | _BV(PRTIM0) | _BV(PRUSI) | _BV(PRADC);
  wait_ms(20);                                  /* boot du ST25DV */

  g_prof = eeprom_read_byte(EE_PROFILE);
  if (g_prof > 3) g_prof = 0;
  g_on = 0;

  led(LED_ALL); wait_ms(100); led(0);           /* flash blanc a la mise sous tension */

  uint8_t ok = 0;
  for (uint8_t i = 0; i < 5 && !ok; i++) ok = rf_set(1);   /* demarre en OFF : NFC coupe */
  if (!ok) fail();
}

void loop(void) {
  if (btn()) { handle_press(); return; }
  if (g_wdt) { g_wdt = 0; if (g_on) heartbeat(); }
  sleep_until_event();
}

#ifndef ARDUINO
int main(void) { setup(); for (;;) loop(); }
#endif
