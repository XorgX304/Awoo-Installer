#include <cstring>
#include <sstream>
#include <filesystem>

#include "install/install_nsp.hpp"
#include "nx/fs.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "util/error.hpp"

#include "ui/MainApplication.hpp"
#include "nspInstall.hpp"
#include "util/config.hpp"
#include "util/util.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;

    void setTopInstInfoText(std::string ourText){
        mainApp->instpage->pageInfoText->SetText(ourText);
        mainApp->CallForRender();
    }

    void setInstInfoText(std::string ourText){
        mainApp->instpage->installInfoText->SetText(ourText);
        mainApp->CallForRender();
    }

    void setInstBarPerc(double ourPercent){
        mainApp->instpage->installBar->SetVisible(true);
        mainApp->instpage->installBar->SetProgress(ourPercent);
        mainApp->CallForRender();
    }

    void loadMainMenu(){
        mainApp->LoadLayout(mainApp->mainPage);
    }

    void loadInstallScreen(){
        mainApp->instpage->pageInfoText->SetText("");
        mainApp->instpage->installInfoText->SetText("");
        mainApp->instpage->installBar->SetProgress(0);
        mainApp->instpage->installBar->SetVisible(false);
        if (inst::config::gayMode) mainApp->instpage->awooImage->SetVisible(false);
        else mainApp->instpage->awooImage->SetVisible(true);
        mainApp->LoadLayout(mainApp->instpage);
        mainApp->CallForRender();
    }
}

namespace nspInstStuff {
    FsStorageId m_destStorageId = FsStorageId_SdCard;

    void installNspFromFile(std::string ourNsp, int whereToInstall)
    {
        appletLockExit();
        inst::ui::loadInstallScreen();
        std::vector<std::string> installList;
        bool nspInstalled = false;
        installList.push_back(ourNsp);

        if (whereToInstall) m_destStorageId = FsStorageId_NandUser;

        for (unsigned int i = 0; i < installList.size(); i++)
        {
            std::string path = "@Sdcard://" + installList[i];

            try
            {
                inst::ui::setTopInstInfoText("Installing " + ourNsp + "...");

                nx::fs::IFileSystem fileSystem;
                fileSystem.OpenFileSystemWithId(path, FsFileSystemType_ApplicationPackage, 0);
                tin::install::nsp::SimpleFileSystem simpleFS(fileSystem, "/", path + "/");
                tin::install::nsp::NSPInstallTask task(simpleFS, m_destStorageId, inst::config::ignoreReqVers);

                printf("Preparing installation\n");
                inst::ui::setInstInfoText("Preparing installation...");
                task.Prepare();

                task.Begin();
                nspInstalled = true;
            }
            catch (std::exception& e)
            {
                printf("Failed to install NSP");
                printf("%s", e.what());
                fprintf(stdout, "%s", e.what());
                inst::ui::mainApp->CreateShowDialog("Failed to install NSP!", "Partially installed NSP contents can be removed from the System Settings applet.\n\n" + (std::string)e.what(), {"OK"}, true);
            }
        }

        if(nspInstalled) if(inst::ui::mainApp->CreateShowDialog(inst::util::shortenString(ourNsp, 64, true) + " installed! Delete NSP from SD card?", "", {"No","Yes"}, false) == 1) std::filesystem::remove("sdmc:/" + ourNsp);

        printf("Done");
        appletUnlockExit();
        inst::ui::loadMainMenu();
        return;
    }
}
