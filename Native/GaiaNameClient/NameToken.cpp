#include "NameToken.hpp"

#include <utility>
#include "NameClient.hpp"

namespace Gaia::NameService
{
    /// Construct and bind the name.
    NameToken::NameToken(NameClient* host, std::string name) noexcept:
        Host(host), Name(std::move(name))
    {
        if (Host)
        {
            Host->RegisterName(Name);
        }
    }

    /// Destruct and notify the host client to unregister the bound name.
    NameToken::~NameToken()
    {
        StopBackgroundUpdater();
        if (Host)
        {
            Host->UnregisterName(Name);
        }
    }

    /// Update the timestamp of this name to keep it valid.
    void NameToken::Update()
    {
        if (Host)
        {
            Host->UpdateName(Name);
        }
    }

    /// Start the background updater thread.
    void NameToken::StartBackgroundUpdater()
    {
        if (UpdaterFlag) return;
        // Start the background updater thread.
        UpdaterFlag = true;
        UpdaterToken = std::async(std::launch::async, [this](){
            while (this->UpdaterFlag)
            {
                // Update the timestamp of names.
                this->Update();
                // Update interval time is 1.5s.
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            }
        });
    }

    /// Stop the background updater thread.
    void NameToken::StopBackgroundUpdater()
    {
        // Stop the background updater thread.
        if (UpdaterFlag)
        {
            UpdaterFlag = false;
            if (UpdaterToken.valid())
            {
                UpdaterToken.get();
            }
        }
    }

    /// Check whether the background updater is running or not.
    bool NameToken::IsBackgroundUpdaterRunning()
    {
        return UpdaterFlag;
    }
}