#include "ravines.h"

#include "rng.h" //rng.h from cubiomes
#include <math.h>

#define PI 3.14159265358979323846

void initCarverSeed(uint64_t *rand, uint64_t worldSeed, int chunkX, int chunkZ)
{
    setSeed(rand, worldSeed);
    setSeed(rand, ((uint64_t)chunkX) * nextLong(rand) ^ ((uint64_t)chunkZ) * nextLong(rand) ^ worldSeed);
}

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