/* simple totp generator
 * generates 6 digit code with 30sec step
 * build with -loath, requires liboath from
 * https://www.nongnu.org/oath-toolkit/ */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <liboath/oath.h>

#include "totp.h"

int generate_totp(const char *key, int keylen, char *code, int digits)
{
        char *secret; /* decoded secret */
        size_t secretlen = 0;
        int rc;
        time_t now;

        rc = oath_init();
        if (rc != OATH_OK)
            return rc;

        rc = oath_base32_decode(key, keylen,
                                &secret, &secretlen);
        if(rc != OATH_OK)
            return rc;

        now = time(NULL);
        rc = oath_totp_generate(secret, secretlen, now,
                                OATH_TOTP_DEFAULT_TIME_STEP_SIZE,
                                OATH_TOTP_DEFAULT_START_TIME,
                                digits, code);
        if (rc != OATH_OK)
            return rc;

//        memset(secret, 0, sizeof(secret));
//        free(secret);

        rc = oath_done();
        if (rc != OATH_OK)
            return rc; /* wtf? */

        return OATH_OK;
}
