#pragma once
#include <string>
#include <iostream>
#include <string>
#include <cstdarg>  // f�r printf-Kompatibilit�t
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
                    return;  // Wenn der Schl�ssel nicht �bereinstimmt, beenden
                }
            }
            else {
                return;  // Wenn die Umgebungsvariable nicht gesetzt ist, beenden
            }

            // Formatierung des Strings
            std::ostringstream oss;
            oss << text;  // Basis-Text anh�ngen

            // Verwendung einer Initializer-Liste, um die Argumente hinzuzuf�gen
            (void)std::initializer_list<int>{ (oss << args, 0)... };

            std::cout << oss.str() << std::endl;  // Ausgabe des formatierten Strings
        }
    };
}
