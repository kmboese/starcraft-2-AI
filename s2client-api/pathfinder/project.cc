#include "sc2api/sc2_api.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2renderer/sc2_renderer.h"

#include "common.h"
#include "flocking.h"

#include <iostream>
// #include <conio.h>
/* NOTE: for some reason, using local scoping for the sc2 namespace in project.cc causes cmake to fail, so I'm skipping it for now.*/

int main(int argc, char* argv[]) {
    sc2::Coordinator coordinator;
    if (!coordinator.LoadSettings(argc, argv)) {
        std::cout << "Unable to find or parse settings." << std::endl;
        return 1;
    }
    //Set game to run realtime (capped framerate) or uncapped
    coordinator.SetRealtime(true);

    //RenderSettings settings(kMapX, kMapY, kMiniMapX, kMiniMapY);
    //coordinator.SetRender(settings);
    
    //Test
    HelloWorld();

#if defined(__linux__)
#if LINUX_USE_SOFTWARE_RENDER
    coordinator.AddCommandLine("-osmesapath /usr/lib/x86_64-linux-gnu/libOSMesa.so");
#else
    coordinator.AddCommandLine("-eglpath /usr/lib/nvidia-384/libEGL.so");
#endif
#endif

    sc2::PathingBot bot;

    coordinator.SetParticipants({
        sc2::CreateParticipant(sc2::Race::Terran, &bot),
        sc2::CreateComputer(sc2::Race::Zerg)
        });


    // Start the game.
    coordinator.LaunchStarcraft();
    //coordinator.StartGame(sc2::kMapBelShirVestigeLE);
    //coordinator.StartGame("D:\\Games\\Starcraft II\\StarCraft II\\Maps\\Melee\\Simple64.SC2Map");
    //Use this as baseline map for pathing marines
    coordinator.StartGame("DefeatRoaches.SC2Map");

    while (coordinator.Update()) {
        if (sc2::PollKeyPress()) {
            break;
        }
    }

    return 0;
}