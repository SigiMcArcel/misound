#pragma once
#include <string>
#include <iostream>
#include <string>
#include <cstdarg>  // für printf-Kompatibilität
#include <sstream>
#include <memory>

namespace misound
{
    static const char* MISOUND_DEBUG_VAR = "MISOUND_DEBUG";
    class miSoundDebug
    {
       
    public:
        template<typename... Args>
        static void miDebug(const std::string& key, const std::string& text, Args... args)
        {
            const char* value = std::getenv(MISOUND_DEBUG_VAR);

            if (value) {
                if (value != key) {
                    return;  // Wenn der Schlüssel nicht übereinstimmt, beenden
                }
            }
            else {
                return;  // Wenn die Umgebungsvariable nicht gesetzt ist, beenden
            }

            // Formatierung des Strings
            std::ostringstream oss;
            oss << text;  // Basis-Text anhängen

            // Verwendung einer Initializer-Liste, um die Argumente hinzuzufügen
            (void)std::initializer_list<int>{ (oss << args, 0)... };

            std::cout << oss.str() << std::endl;  // Ausgabe des formatierten Strings
        }
    };
}
