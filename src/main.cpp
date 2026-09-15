#include "pch.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <string>

namespace
{
    constexpr int DEFAULT_BLOCK_KEY = 47;  // V

    constexpr auto MCM_DEFAULT_INI =
        "Data\\MCM\\Config\\SimpleDualWieldBlock\\settings.ini";

    constexpr auto MCM_USER_INI =
        "Data\\MCM\\Settings\\SimpleDualWieldBlock.ini";

    bool g_blocking = false;
    int g_blockKey = DEFAULT_BLOCK_KEY;


    std::string Trim(const std::string& a_string)
    {
        const auto first = std::find_if_not(
            a_string.begin(),
            a_string.end(),
            [](unsigned char c) {
                return std::isspace(c);
            });

        const auto last = std::find_if_not(
            a_string.rbegin(),
            a_string.rend(),
            [](unsigned char c) {
                return std::isspace(c);
            }).base();

        if (first >= last) {
            return {};
        }

        return std::string(first, last);
    }


    bool ReadIniValue(
        const char* a_path,
        const std::string& a_section,
        const std::string& a_key,
        std::string& a_result)
    {
        std::ifstream file(a_path);

        if (!file.is_open()) {
            return false;
        }

        std::string line;
        std::string currentSection;

        while (std::getline(file, line)) {
            line = Trim(line);

            if (line.empty()) {
                continue;
            }

            if (line[0] == '#' || line[0] == ';') {
                continue;
            }

            const auto sectionStart = line.find('[');
            const auto sectionEnd = line.find(']');

            if (sectionStart != std::string::npos &&
                sectionEnd != std::string::npos &&
                sectionEnd > sectionStart)
            {
                currentSection =
                    Trim(line.substr(
                        sectionStart + 1,
                        sectionEnd - sectionStart - 1));

                continue;
            }

            if (currentSection != a_section) {
                continue;
            }

            const auto equals = line.find('=');

            if (equals == std::string::npos) {
                continue;
            }

            const auto key =
                Trim(line.substr(0, equals));

            if (key != a_key) {
                continue;
            }

            a_result =
                Trim(line.substr(equals + 1));

            return true;
        }

        return false;
    }


    int ReadInt(
        const char* a_section,
        const char* a_key,
        int a_default)
    {
        std::string value;

        if (ReadIniValue(
                MCM_USER_INI,
                a_section,
                a_key,
                value))
        {
            const int result = std::atoi(value.c_str());

            SKSE::log::info(
                "Loaded USER block key: {}",
                result);

            return result;
        }

        if (ReadIniValue(
                MCM_DEFAULT_INI,
                a_section,
                a_key,
                value))
        {
            const int result = std::atoi(value.c_str());

            SKSE::log::info(
                "Loaded DEFAULT block key: {}",
                result);

            return result;
        }

        SKSE::log::warn(
            "Could not read block key, using fallback {}",
            a_default);

        return a_default;
    }


    void LoadSettings()
    {
        g_blockKey =
            ReadInt(
                "General",
                "iBlockKey",
                DEFAULT_BLOCK_KEY);
    }


    class InputEventHandler :
        public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        static InputEventHandler* GetSingleton()
        {
            static InputEventHandler singleton;
            return std::addressof(singleton);
        }


        RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent* const* a_event,
            RE::BSTEventSource<RE::InputEvent*>*) override
        {
            if (!a_event) {
                return RE::BSEventNotifyControl::kContinue;
            }

            auto* player =
                RE::PlayerCharacter::GetSingleton();

            if (!player) {
                return RE::BSEventNotifyControl::kContinue;
            }

            for (auto* event = *a_event;
                 event;
                 event = event->next)
            {
                auto* button =
                    event->AsButtonEvent();

                if (!button) {
                    continue;
                }

                if (button->GetDevice() !=
                    RE::INPUT_DEVICE::kKeyboard)
                {
                    continue;
                }

                LoadSettings();

                if (static_cast<int>(
                        button->GetIDCode()) != g_blockKey)
                {
                    continue;
                }

                auto* actorState =
                    player->AsActorState();

                if (!actorState) {
                    continue;
                }

                if (button->IsDown()) {
                    if (!g_blocking) {
                        actorState->actorState2.wantBlocking = 1;

                        player->NotifyAnimationGraph(
                            "blockStart");

                        g_blocking = true;
                    }
                }

                if (button->IsUp()) {
                    if (g_blocking) {
                        actorState->actorState2.wantBlocking = 0;

                        player->NotifyAnimationGraph(
                            "blockStop");

                        g_blocking = false;
                    }
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }
    };


    void RegisterInput()
    {
        auto* inputManager =
            RE::BSInputDeviceManager::GetSingleton();

        if (!inputManager) {
            return;
        }

        LoadSettings();

        inputManager->AddEventSink(
            InputEventHandler::GetSingleton());
    }


    void MessageHandler(
        SKSE::MessagingInterface::Message* a_message)
    {
        if (!a_message) {
            return;
        }

        if (a_message->type ==
            SKSE::MessagingInterface::kInputLoaded)
        {
            RegisterInput();
        }
    }

} // namespace


SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    auto* messaging =
        SKSE::GetMessagingInterface();

    if (!messaging) {
        return false;
    }

    messaging->RegisterListener(
        MessageHandler);

    return true;
}