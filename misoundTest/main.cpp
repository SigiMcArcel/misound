#include <cstdio>
#include <mi/misound/Audio.h>
#include <mi/misound/Wave.h>

int main()
{
    printf("Hallo aus %s!\n", "misoundlibTest");
    misound::Wave _wave("/home/root/sounds/A2.wav", "A2.wav", true);
   while (true)
   {
       _wave.play();
       ::sleep(10);
       _wave.stop();
   }

    return 0;
}