//
// Created by isaaf on 10/9/2026.
//

#ifndef ENCRYPTION_H
#define ENCRYPTION_H
#include <stdint.h>
uint32_t seed;

///
/// This allows for the encryption to change every clock cycle.
/// Can be disabled by returning the static seed.
///
/// @return The new generated seed
uint32_t clkSyncronization();

///
/// Allows to decrypt the message using the homemade encryption algorythm
///
/// @param msg The encrypted message
/// @return The decrypted string
char* decrypt(char msg[]);

///
/// Encrypts any message using the homemade encryption algorythm
///
/// @param msg The unencrypted message
/// @return the encrypted string
char* encrypt(char msg[]);
#endif //ENCRYPTION_H
