#ifndef TOTP_H
#define TOTP_H

int generate_totp(const char *secret, int keylen, char *code, int digits);

#endif // TOTP_H
