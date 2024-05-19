// This code is public domain
// https://creativecommons.org/public-domain/cc0/

// Created by DuncanRuns

#include "finders.h"
#include <stdio.h>
#include <stdint.h>

#define PI 3.14159265358979323846

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

void initCarverSeed(uint64_t *rand, uint64_t worldSeed, int chunkX, int chunkZ)
{
    setSeed(rand, worldSeed);
    setSeed(rand, ((uint64_t)chunkX) * nextLong(rand) ^ ((uint64_t)chunkZ) * nextLong(rand) ^ worldSeed);
}

/**
 * Initializes a ravine generator.
 * <br>
 * This will create a RavineGenerator with starting parameters and some approximations. To get accurate values, use simulateRavineToMiddle afterwards.
 * <br>
 * The returned struct has the following characteristics:
 * <li> canSpawn == 1 if the ravine can spawn; any of the following values are invalid if canSpawn == 0 </li>
 * <li> yaw, pitch, x, y, and z are initialized to the start of the ravine (these changes over iteration of ravine drawing) </li>
 * <li> ravineLength and verticalRadiusAtCenter are calculated and are accurate</li>
 * <li> xGuess, zGuess, upperY, and lowerY are approximate guesses of the middle of the ravine based on other starting parameters </li>
 */
RavineGenerator initRavine(uint64_t worldSeed, int chunkX, int chunkZ)
{
    RavineGenerator gen;
    uint64_t *rand = &gen.rand;
    initCarverSeed(rand, worldSeed, chunkX, chunkZ);
    if (nextFloat(rand) > 0.02)
    {
        gen.canSpawn = 0;
        return gen;
    }
    gen.canSpawn = 1;

    gen.x = (double)chunkX * 16 + nextInt(rand, 16);
    gen.y = (double)(nextInt(rand, nextInt(rand, 40) + 8) + 20);
    gen.z = (double)(chunkZ * 16 + nextInt(rand, 16));
    gen.yaw = nextFloat(rand) * (float)(PI * 2);
    gen.pitch = (nextFloat(rand) - 0.5F) / 4.0F;
    gen.verticalRadiusAtCenter = (1.5 + (double)((nextFloat(rand) * 2.0F + nextFloat(rand)) * 2.0F)) * 3.0;
    gen.ravineLength = 112 - nextInt(rand, 28);
    // Approximations
    int middleMiddleY = gen.y + (int)(sin(gen.pitch) * gen.ravineLength / 2);
    gen.lowerY = (int)(middleMiddleY - gen.verticalRadiusAtCenter);
    gen.upperY = (int)(middleMiddleY + gen.verticalRadiusAtCenter + 1);
    float deltaHorizontal = cos(gen.pitch);
    gen.xGuess = (int)(gen.ravineLength / 2 * cos(gen.yaw) * deltaHorizontal);
    gen.zGuess = (int)(gen.ravineLength / 2 * sin(gen.yaw) * deltaHorizontal);
    return gen;
}

/**
 * Simulates half of a ravine using a RavineGenerator made from initRavine.
 * <br>
 * The returned struct has the following characteristics:
 * <li> canSpawn is unchanged and should have been checked before </li>
 * <li> xGuess, zGuess, ravineLength and verticalRadiusAtCenter are also unchanged </li>
 * <li> yaw, pitch, x, y, and z represent coordinates (and direction of the ravine) at the center of the ravine </li>
 * <li> upperY and lowerY are set to the y level bounds of the center of the ravine (full bounds may not be reached, allow a few extra blocks if lava is required) </li>
 */
void simulateRavineToMiddle(RavineGenerator *gen)
{
    uint64_t *rand = &gen->rand;
    uint64_t canyonSeed = nextLong(rand);
    setSeed(rand, canyonSeed);
    // Let's roll some rng for fun!!! (It makes some sort of block mask or something)
    for (int fun = 0; fun < 256; ++fun)
    {
        if (fun == 0 || nextInt(rand, 3) == 0)
        {
            nextFloat(rand);
            nextFloat(rand);
        }
    }
    float ravineShifterA = 0.0F;
    float ravineShifterB = 0.0F;

    for (int u = 0; u < (gen->ravineLength / 2); ++u)
    {
        //        printf("%d:%lld - %.5f\n",u,*rand,gen->x);
        float deltaHorizontal = cos(gen->pitch);
        float deltaY = sin(gen->pitch);
        nextFloat(rand);
        nextFloat(rand);
        gen->x += (double)(cos(gen->yaw) * deltaHorizontal);
        gen->y += (double)deltaY;
        gen->z += (double)(sin(gen->yaw) * deltaHorizontal);
        gen->pitch *= 0.7F;
        gen->pitch += ravineShifterB * 0.05F;
        gen->yaw += ravineShifterA * 0.05F;
        ravineShifterB *= 0.8F;
        ravineShifterA *= 0.5F;
        ravineShifterB += (nextFloat(rand) - nextFloat(rand)) * nextFloat(rand) * 2.0F;
        ravineShifterA += (nextFloat(rand) - nextFloat(rand)) * nextFloat(rand) * 4.0F;
        nextInt(rand, 4);
    }

    gen->upperY = (int)(gen->y + gen->verticalRadiusAtCenter + 1);
    // MC code suggests additional -1, but it seems to usually not reach it
    gen->lowerY = (int)(gen->y - gen->verticalRadiusAtCenter);
    if (gen->lowerY < 1)
    {
        gen->lowerY = 1;
    }
    if (gen->upperY > 248)
    {
        gen->upperY = 248;
    }
}

// Test functions below, do not include in project

void generateSomeRavines()
{
    int totalFound = 0;
    for (uint64_t worldSeed = 0; worldSeed < (1 << 16); worldSeed++)
    {
        RavineGenerator r = initRavine(worldSeed, 0, 0);
        if (r.canSpawn)
        {
            int startX = (int)r.x;
            int startY = (int)r.y;
            int startZ = (int)r.z;
            int gx = r.xGuess, gz = r.zGuess;
            int lowerGuess = r.lowerY;
            int upperGuess = r.upperY;
            simulateRavineToMiddle(&r);
            if (r.lowerY < 8 && r.upperY >= 41)
            {
                printf("%lld: start=(%d %d %d), guess=(%d %d->%d %d), middle=(%d %d %d), middleRange=(%d,%d)\n",
                       worldSeed,
                       startX, startY, startZ,
                       gx, lowerGuess, upperGuess, gz,
                       (int)r.x, (int)r.y, (int)r.z,
                       r.lowerY, r.upperY);
                if (++totalFound == 10)
                {
                    return;
                }
            }
        }
    }
}

void findSeedsWithMagmaRavines()
{
    Generator g;
    setupGenerator(&g, MC_1_16_1, 0);
    int totalFound = 0;
    for (uint64_t lower48 = 0; lower48 < (1 << 16); lower48++)
    {
        RavineGenerator r = initRavine(lower48, 0, 0);
        if (r.canSpawn)
        {
            simulateRavineToMiddle(&r);
            if (r.lowerY > 7 || r.upperY < 42)
                continue;
            for (uint64_t upper16 = 0; upper16 < 300; upper16++)
            {
                uint64_t worldSeed = (upper16 << 48) | lower48;
                applySeed(&g, DIM_OVERWORLD, worldSeed);
                int i = getBiomeAt(&g, 4, r.x / 4, r.y / 4, r.z / 4);
                if (isDeepOcean(i))
                {
                    printf("%lld: %d %d %d\n", worldSeed, (int)r.x, (int)r.y, (int)r.z);
                    if (++totalFound == 100)
                    {
                        return;
                    }
                    break;
                }
            }
        }
    }
}
