#ifndef DUNCANRUNS_RAVINES_H_
#define DUNCANRUNS_RAVINES_H_

#include <stdint.h>

typedef struct
{
    uint64_t rand;
    uint8_t canSpawn;
    int xGuess, zGuess;
    int upperY, lowerY;
    float yaw, pitch;
    int ravineLength;
    double verticalRadiusAtCenter;
    double x, y, z;
} RavineGenerator;

void initCarverSeed(uint64_t *rand, uint64_t worldSeed, int chunkX, int chunkZ);

/**
 * Initializes a ravine generator.
 *
 * This will create a RavineGenerator with starting parameters and some approximations. To get accurate values, use simulateRavineToMiddle afterwards.
 *
 * The returned struct has the following characteristics:
 * - canSpawn == 1 if the ravine can spawn; any of the following values are invalid if canSpawn == 0
 * - yaw, pitch, x, y, and z are initialized to the start of the ravine (these changes over iteration of ravine drawing)
 * - ravineLength and verticalRadiusAtCenter are calculated and are accurate
 * - xGuess, zGuess, upperY, and lowerY are approximate guesses of the middle of the ravine based on other starting parameters
 */
RavineGenerator initRavine(uint64_t worldSeed, int chunkX, int chunkZ);

/**
 * Simulates half of a ravine using a RavineGenerator made from initRavine.
 *
 * The returned struct has the following characteristics:
 * - canSpawn is unchanged and should have been checked before
 * - xGuess, zGuess, ravineLength and verticalRadiusAtCenter are also unchanged
 * - yaw, pitch, x, y, and z represent coordinates (and direction of the ravine) at the center of the ravine
 * - upperY and lowerY are set to the y level bounds of the center of the ravine (full bounds may not be reached, allow a few extra blocks if lava is required)
 */
void simulateRavineToMiddle(RavineGenerator *gen);

#endif // DUNCANRUNS_RAVINES_H_