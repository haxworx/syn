void
effect_normal(int16_t *sound, int count)
{
   int effect_count = 0;
   while (*sound > 0 && effect_count < 1264)
     {
        *sound -= (count * 2);
        ++effect_count;
     }
   effect_count = 0;
}

void
effect_triangle(int16_t *sound, int count)
{
   int effect_count = 0;
   while (*sound > 0 && effect_count < 64)
     {
        *sound += (count * 3);
        ++effect_count;
     }
}

void
effect_girls_n_boys(int16_t *sound, int count)
{
   if (*sound > 0)
     *sound += 3 * count;
}

void
effect_drum_n_bass(int16_t *sound, int count)
{
   *sound += 3 * count;
}

void
effect_creepy_fuzz(int16_t *sound, int count)
{
   if (*sound < 10)
     *sound += (3 * count);
   if (*sound > 100)
     *sound = 0xfa;
}

void
effect_bassy(int16_t *sound, int count)
{
   if (!(count % 4))
     *sound *= -1;

   *sound >>= 4;
}

void
effect_bowser(int16_t *sound, int count)
{
   if (!(count % 4))
     *sound *= -1;
   while (*sound > 0)
     *sound -= 0xa;
}

void
effect_vinyl_scratch(int16_t *sound, int count)
{
   while (*sound < 0)
     *sound += 0xf;

   while (*sound > 0)
     *sound *= 256;
}

void
effect_lazer_quest(int16_t *sound, int count)
{
   *sound *= tan(4096 * 1024 * atan(count * 4));
}

