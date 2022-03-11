#include <codecvt>
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>

#pragma execution_character_set( "utf-8" )

using std::locale;
using std::codecvt_utf8;

using std::ios;
using std::wcout;
using std::setw;
using std::setprecision;
using std::fixed;

using std::string;
using std::getline;
using std::size;

using std::wstring;
using std::wifstream;
using std::wofstream;

constexpr int LUNGIME_MAXIMA_STRING = 1024;

constexpr char CALE_CATRE_SETURI_DE_DATE[] = "seturi_de_date/\0";
constexpr char SETURI_DE_DATE[][LUNGIME_MAXIMA_STRING] = {
    "ethereum_whitepaper\0", "ion_volumul_i\0", "ion_volumul_ii\0"
};
constexpr char EXTENSIE_SETURI_DE_DATE[] = "txt\0";

constexpr wchar_t CARACTERE_PERMISE[] = {
    L'A', L'a',
    L'Ă', L'ă',
    L'Â', L'â',
    L'B', L'b',
    L'C', L'c',
    L'D', L'd',
    L'E', L'e',
    L'F', L'f',
    L'G', L'g',
    L'H', L'h',
    L'I', L'i',
    L'Î', L'î',
    L'J', L'j',
    L'K', L'k',
    L'L', L'l',
    L'M', L'm',
    L'N', L'n',
    L'O', L'o',
    L'P', L'p',
    L'Q', L'q',
    L'R', L'r',
    L'S', L's',
    L'Ș', L'ș',
    L'T', L't',
    L'Ț', L'ț',
    L'U', L'u',
    L'V', L'v',
    L'W', L'w',
    L'X', L'x',
    L'Y', L'y',
    L'Z', L'z',
};
constexpr int NUMAR_CARACTERE_PERMISE = size(CARACTERE_PERMISE);
constexpr int NEGASIT = -1;

const locale UTF8(locale::empty(), new codecvt_utf8<wchar_t>);

const auto PRECIZIE = setprecision(21);
const auto SEPARATOR_SETURI_DE_DATE = wstring(100, L'=');
const auto SEPARATOR_LITERE = wstring(70, L'-');

const wstring TEXT_TOTAL = L"***       Total -> ";
const auto LATIME_TOTAL = setw(1 + TEXT_TOTAL.length());

wifstream g_fisier;

constexpr char CALE_CATRE_RAPOARTE[] = "rapoarte/\0";
constexpr char EXTENSIE_RAPORT[] = "txt\0";

/**
 * @param p_set_de_date Numele setului de date
 * @returns Calea relativă către setul de date
 */
string
GetCaleCatreSetDeDate(const char* p_set_de_date)
{
    auto cale = string(CALE_CATRE_SETURI_DE_DATE);
    cale.append(p_set_de_date);
    cale.append(1, '.');
    cale.append(EXTENSIE_SETURI_DE_DATE);
    return cale;
}

/**
 * @param p_set_de_date Numele setului de date
 * @returns Calea relativă către raport
 */
string
GetCaleCatreRaport(const char* p_set_de_date)
{
    auto cale = string(CALE_CATRE_RAPOARTE);
    cale.append(p_set_de_date);
    cale.append(1, '.');
    cale.append(EXTENSIE_RAPORT);
    return cale;
}

/**
 * @param p_frecvente Tabloul de secvențe
 * @returns Numărul total de caractere permise în raport cu tabloul furnizat
 */
int
GetNumarTotalCaracterePermise(const int* p_frecvente)
{
    int numar_total = 0;
    for (int index = 0; index < NUMAR_CARACTERE_PERMISE; index++)
    {
        numar_total += p_frecvente[index];
    }
    return numar_total;
}

/**
 * @param p_numar Numarul pentru care se dorește aflarea numărului de cifre
 * @returns Număul de cifre ale numărului furnizat
 */
int
GetNumarCifre(int p_numar)
{
    int numar_cifre = 0;
    while (p_numar)
    {
        numar_cifre++;
        p_numar /= 10;
    }
    return numar_cifre;
}

/**
 * @param p_caracter Caracterul căutat
 * @returns Indexul caracterului căutat în tabloul cu caractere permise
 */
int
GetIndexCaracterPermis(const wchar_t& p_caracter)
{
    for (int index = 0; index < NUMAR_CARACTERE_PERMISE; index++)
    {
        if (CARACTERE_PERMISE[index] == p_caracter)
        {
            return index;
        }
    }
    return NEGASIT;
}

/**
 * Afișarea în consolă a rezultatelor procesării setului de date curent
 * @param p_frecvente Tabloul de secvențe
 * @param p_set_de_date Numele setului de date - pentru afișare
 */
void
AfisareRezultate(const int* p_frecvente, const char* p_set_de_date)
{
    wcout << L"\n\n" << SEPARATOR_SETURI_DE_DATE;

    const auto numar_caractere = GetNumarTotalCaracterePermise(p_frecvente);
    wcout << L"\n\nFrecvențele celor " << numar_caractere <<
        L" de caractere ale alfabetului românesc din setul de date \"" << p_set_de_date << L"\":\n";

    long double progres = 0;

    // Această buclă este safe deoarece știu că am un număr par de chei în Tablou
    for (int index = 0; index < NUMAR_CARACTERE_PERMISE; index += 2)
    {
        const auto flaguri_originale = wcout.flags();

        wcout << L"\n" << SEPARATOR_LITERE;

        const auto total = p_frecvente[index] + p_frecvente[index + 1];
        const auto latime = setw(GetNumarCifre(total));

        const auto probabilitate_uppercase = static_cast<long double>(p_frecvente[index]) / numar_caractere;
        const auto probabilitate_lowercase = static_cast<long double>(p_frecvente[index + 1]) / numar_caractere;
        const auto probabilitate_total = static_cast<long double>(total) / numar_caractere;
        progres += probabilitate_total;

        wcout << L"\n" << LATIME_TOTAL << CARACTERE_PERMISE[index] << L": " << latime << p_frecvente[index] <<
            L" - Probabilitate: " << fixed << PRECIZIE << probabilitate_uppercase;
        wcout << L"\n\n" << LATIME_TOTAL << CARACTERE_PERMISE[index + 1] << L": " << latime << p_frecvente[index + 1] <<
            L" - Probabilitate: " << fixed << PRECIZIE << probabilitate_lowercase;
        wcout << L"\n\n" << TEXT_TOTAL << CARACTERE_PERMISE[index] << ": " << total << L" - Probabilitate: " << fixed <<
            PRECIZIE << probabilitate_total;
        wcout << L"\n\n*** Cod Huffman -> 0110101010";

        wcout << L"\n" << SEPARATOR_LITERE << L"\n\n";

        wcout.flags(flaguri_originale);

        wcout << L"+++++ Progres: " << PRECIZIE << progres << L" / 1 +++++\n";
    }

    wcout << L"\n" << SEPARATOR_SETURI_DE_DATE << L"\n\n";
}

/**
 * Bucla principală a programului care procesează setul de date curent
 * @param p_set_de_date Numele setului de date - pentru afișare
 */
void
ProcesareSetDeDate(const char* p_set_de_date)
{
    int frecvente[NUMAR_CARACTERE_PERMISE] = {0};

    wstring linie;
    while (getline(g_fisier, linie))
    {
        for (const auto caracter : linie)
        {
            const auto index_caracter = GetIndexCaracterPermis(caracter);
            if (index_caracter == NEGASIT)
            {
                // Caracterul nu este permis
                continue;
            }
            frecvente[index_caracter]++;
        }
    }

    AfisareRezultate(frecvente, p_set_de_date);
}

int
main(void)
{
    SetConsoleOutputCP(65001);
    _setmode(_fileno(stdout), _O_U16TEXT);

    for (const auto& set_de_date : SETURI_DE_DATE)
    {
        g_fisier.open(GetCaleCatreSetDeDate(set_de_date));
        g_fisier.imbue(UTF8);

        ProcesareSetDeDate(set_de_date);
        
        g_fisier.close();
    }

    return EXIT_SUCCESS;
}
