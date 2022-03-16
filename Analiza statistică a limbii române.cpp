#include <codecvt>
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>

#pragma execution_character_set("utf-8")

using std::locale;
using std::codecvt_utf8;

using std::wcout;
using std::setw;
using std::setprecision;
using std::fixed;

using std::getline;

using std::wstring;
using std::wifstream;
using std::vector;
using std::priority_queue;

using std::logic_error;
using std::log2;

const auto CALE_CATRE_SETURI_DE_DATE = wstring(L"seturi_de_date/");
const auto SETURI_DE_DATE = {
    wstring(L"ethereum_whitepaper"),
    wstring(L"amintiri_din_copilarie")
};
const auto EXTENSIE_SETURI_DE_DATE = wstring(L"txt");

const auto CARACTERE_PERMISE = wstring({
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
});
const size_t NUMAR_CARACTERE_PERMISE = CARACTERE_PERMISE.length();

const locale UTF8(locale::empty(), new codecvt_utf8<wchar_t>);

const auto PRECIZIE = setprecision(21);
const auto SEPARATOR_SETURI_DE_DATE = wstring(100, L'=');
const auto SEPARATOR_LITERE = wstring(70, L'-');

const auto TEXT_TOTAL = wstring(L"***       Total -> ");
const auto LATIME_TOTAL = setw(1 + TEXT_TOTAL.length());

wifstream g_fisier;

const auto CARACTER_NOD_INTERMEDIAR = L'#';

/**
 * Structură pentru a conține datele din nodurile arborelui de coduri Huffmanw
 */
struct CelulaHuffman
{
    wchar_t caracter;
    size_t frecventa;
    CelulaHuffman* stanga;
    CelulaHuffman* dreapta;

    CelulaHuffman(const wchar_t& p_caracter,
                  const size_t& p_frecventa)
        : caracter(p_caracter),
          frecventa(p_frecventa),
          stanga(nullptr),
          dreapta(nullptr)
    {
    }

    ~CelulaHuffman()
    {
        delete stanga;
        delete dreapta;
    }
};

typedef CelulaHuffman* NodHuffman;

/**
 * Structură comparator (mai formal se numește predicat binar) care se folosește pentru <see cref="https://www.cplusplus.com/reference/queue/priority_queue/">priority queue</see>
 */
struct ComparatorHuffman
{
    /**
     * @param p_nod_1 Primul nod (generic)
     * @param p_nod_2 Al doilea nod (generic)
     * @returns Un boolean care dacă este "true" înseamnă că p_nod_1 trebuie să fie înaintea lui p_nod_2, iar dacă este fals,
     * p_nod_1 trebuie să fie după p_nod_2
     */
    bool
    operator()(NodHuffman p_nod_1, NodHuffman p_nod_2) const
    {
        return p_nod_1->frecventa > p_nod_2->frecventa;
    }
};

typedef priority_queue<NodHuffman, vector<NodHuffman>, ComparatorHuffman> HuffmanHeap;

/**
 * Structură auxiliară folosită pentru a menține ordinea codurilor Huffman
 */
struct PerecheCaracterCodHuffman
{
    wchar_t caracter;
    wstring cod_huffman;
};

/**
 * Structură comparator (mai formal se numește predicat binar) care se folosește pentru <see cref="https://www.cplusplus.com/reference/queue/priority_queue/">priority queue</see>
 */
struct ComparatorPerecheCaracterCodHuffman
{
    /**
     * @param p_pereche_1 Prima pereche (generică)
     * @param p_pereche_2 A doua pereche (generică)
     * @returns Un boolean care dacă este "true" înseamnă că p_pereche_1 trebuie să fie înaintea lui p_pereche_2, iar dacă este fals,
     * p_pereche_1 trebuie să fie după p_pereche_2
     */
    bool
    operator()(const PerecheCaracterCodHuffman& p_pereche_1, const PerecheCaracterCodHuffman& p_pereche_2) const
    {
        const auto pozitia_caracterului_din_prima_pereche = CARACTERE_PERMISE.find(p_pereche_1.caracter);
        const auto pozitia_caracterului_din_a_doua_pereche = CARACTERE_PERMISE.find(p_pereche_2.caracter);

        if (pozitia_caracterului_din_prima_pereche == wstring::npos || pozitia_caracterului_din_a_doua_pereche ==
            wstring::npos)
        {
            throw logic_error("Unul dintre caractere nu se află printre caracterele permise!");
        }

        return pozitia_caracterului_din_prima_pereche > pozitia_caracterului_din_a_doua_pereche;
    }
};

typedef priority_queue<PerecheCaracterCodHuffman, vector<PerecheCaracterCodHuffman>,
                       ComparatorPerecheCaracterCodHuffman> CoadaPerecheHuffman;

/**
 * @param p_set_de_date Numele setului de date
 * @returns Calea relativă către setul de date
 */
wstring
GetCaleCatreSetDeDate(const wstring& p_set_de_date)
{
    auto cale = wstring(CALE_CATRE_SETURI_DE_DATE);
    cale.append(p_set_de_date);
    cale.append(1, '.');
    cale.append(EXTENSIE_SETURI_DE_DATE);
    return cale;
}

/**
 * @param p_numar Numarul pentru care se dorește aflarea numărului de cifre
 * @returns Număul de cifre ale numărului furnizat
 */
size_t
GetNumarCifre(size_t p_numar)
{
    size_t numar_cifre = 0;
    while (p_numar)
    {
        numar_cifre++;
        p_numar /= 10;
    }
    return numar_cifre;
}

/**
 * @param p_varf Adresa vârfului heap-ului procesat
 * @param p_cod Codul curent (parametru folosit la recursivitate)
 * @returns Un priority list de pair-uri de tip (caracter, cod) - pentru a menține ordinea (CASE INSENSITIVE)
 */
CoadaPerecheHuffman
GetCoadaCoduriHuffman(NodHuffman p_varf, const wstring& p_cod)
{
    if (p_varf == nullptr)
    {
        return {};
    }

    CoadaPerecheHuffman coada_finala;

    CoadaPerecheHuffman coada_stanga = GetCoadaCoduriHuffman(p_varf->stanga, p_cod + L'0');
    CoadaPerecheHuffman coada_dreapta = GetCoadaCoduriHuffman(p_varf->dreapta, p_cod + L'1');
    if (p_varf->caracter != CARACTER_NOD_INTERMEDIAR)
    {
        coada_finala.push({p_varf->caracter, p_cod});
    }

    // Nu contează ordinea în care le inserăm datorită faptului că e priority queue și am stabilit ordinea în comparator
    while (!coada_stanga.empty())
    {
        coada_finala.push(coada_stanga.top());
        coada_stanga.pop();
    }

    while (!coada_dreapta.empty())
    {
        coada_finala.push(coada_dreapta.top());
        coada_dreapta.pop();
    }

    return coada_finala;
}

/**
 * Crează un tablou în funcție de priority queue. Fac acest lucru deoarece este redundant să mai avem și caracterele când deja le știm ordinea
 * - ele au fost folosite doar în scopul menținerii ordinii.
 * @param p_coada Priority queue procesat
 * @returns Un tablou de string-uri care reprezintă codurile Huffman în ordine
 */
vector<wstring>
GetTablouCoduriHuffman(CoadaPerecheHuffman p_coada)
{
    vector<wstring> ret;
    while (!p_coada.empty())
    {
        ret.push_back(p_coada.top().cod_huffman);
        p_coada.pop();
    }
    return ret;
}

/**
 * @param p_frecvente Tabloul de frecvențe ale caracterelor
 * @param p_numar_caractere Parametru prin referință pentru a elimina redundanța în calcularea numărului total de caractere deoarce știm că acesta este vârful heap-ului Huffman
 * @returns Un tablou cu codurile Huffman aferente caracterelor (CASE INSENSITIVE)
 */
vector<wstring>
GetCoduriHuffman(const vector<size_t>& p_frecvente, long double& p_numar_caractere)
{
    NodHuffman varf = nullptr;

    HuffmanHeap heap;

    for (size_t index = 0; index < p_frecvente.size(); index += 2)
    {
        heap.push(new CelulaHuffman(CARACTERE_PERMISE[index], p_frecvente[index] + p_frecvente[index + 1]));
    }

    while (heap.size() > 1)
    {
        NodHuffman stanga = heap.top();
        heap.pop();

        NodHuffman dreapta = heap.top();
        heap.pop();

        varf = new CelulaHuffman(CARACTER_NOD_INTERMEDIAR, stanga->frecventa + dreapta->frecventa);
        varf->stanga = stanga;
        varf->dreapta = dreapta;
        heap.push(varf);
    }

    p_numar_caractere = static_cast<long double>(varf->frecventa);
    const auto coada = GetCoadaCoduriHuffman(varf, L"");

    delete varf;

    return GetTablouCoduriHuffman(coada);
}

/**
 * Afișarea în consolă a rezultatelor procesării setului de date curent
 * @param p_frecvente Tabloul de secvențe
 * @param p_set_de_date Numele setului de date - pentru afișare
 */
void
AfisareRezultate(const vector<size_t>& p_frecvente, const wstring& p_set_de_date)
{
    auto numar_caractere = 0.0L;
    const auto coduri_huffman = GetCoduriHuffman(p_frecvente, numar_caractere);

    wcout << SEPARATOR_SETURI_DE_DATE;

    wcout << L"\n\nFrecvențele celor " << numar_caractere <<
        L" de caractere ale alfabetului românesc din setul de date \"" << p_set_de_date << L"\":\n";

    auto progres = 0.0L;
    auto entropie = 0.0L;
    auto lungimea_medie_a_combinatiilor_de_cod = 0.0L;

    for (size_t index = 0; index < NUMAR_CARACTERE_PERMISE; index += 2)
    {
        const auto flaguri_originale = wcout.flags();

        wcout << L"\n" << SEPARATOR_LITERE;

        const auto total = p_frecvente[index] + p_frecvente[index + 1];
        const auto latime = setw(GetNumarCifre(total));

        const auto probabilitate_uppercase = static_cast<long double>(p_frecvente[index]) / numar_caractere;
        const auto probabilitate_lowercase = static_cast<long double>(p_frecvente[index + 1]) / numar_caractere;
        const auto probabilitate_total = static_cast<long double>(total) / numar_caractere;
        progres += probabilitate_total;

        const auto cod_huffman = coduri_huffman[index / 2];

        wcout << L"\n" << LATIME_TOTAL << CARACTERE_PERMISE[index] << L": " << latime << p_frecvente[index] <<
            L" - Probabilitate: " << fixed << PRECIZIE << probabilitate_uppercase;
        wcout << L"\n\n" << LATIME_TOTAL << CARACTERE_PERMISE[index + 1] << L": " << latime << p_frecvente[index + 1] <<
            L" - Probabilitate: " << fixed << PRECIZIE << probabilitate_lowercase;
        wcout << L"\n\n" << TEXT_TOTAL << CARACTERE_PERMISE[index] << ": " << total << L" - Probabilitate: " << fixed <<
            PRECIZIE << probabilitate_total;
        wcout << L"\n\n*** Cod Huffman -> " << cod_huffman;

        wcout << L"\n" << SEPARATOR_LITERE << L"\n\n";

        wcout.flags(flaguri_originale);

        wcout << L"+++++ Progres valoare unitară: " << PRECIZIE << progres << L" / 1 +++++\n";

        lungimea_medie_a_combinatiilor_de_cod += probabilitate_total * static_cast<long double>(cod_huffman.length());
        if (probabilitate_total > 0.0L)
        {
            entropie += probabilitate_total * log2(1 / probabilitate_total);
        }
    }

    wcout << L"\n" << SEPARATOR_SETURI_DE_DATE << L"\n";

    const auto eficienta = entropie / lungimea_medie_a_combinatiilor_de_cod;
    const auto redundanta = 1 - eficienta;

    wcout << L"\nEntropia (biți/caracter) = " << entropie << L"\n";
    wcout << L"\nM = 2 (folosim cod binar)\n";
    wcout << L"\nLungimea medie a combinațiilor de cod (biți/caracter) = " << lungimea_medie_a_combinatiilor_de_cod << L"\n";
    wcout << L"\nEficiența = " << eficienta << L"\n";
    wcout << L"\nRedundanța = " << redundanta << L"\n";

    wcout << L"\n" << entropie << L" ≤ " << lungimea_medie_a_combinatiilor_de_cod << L" ≤ " << entropie + 1;
    wcout << L"\nTeorema I a lui Shannon este verificată!\n\n";
}

/**
 * Bucla principală a programului care procesează setul de date curent
 * @param p_set_de_date Numele setului de date - pentru afișare
 */
void
ProcesareSetDeDate(const wstring& p_set_de_date)
{
    vector<size_t> frecvente(NUMAR_CARACTERE_PERMISE, 0);

    wstring linie;
    while (getline(g_fisier, linie))
    {
        for (const auto& caracter : linie)
        {
            const auto index_caracter = CARACTERE_PERMISE.find(caracter);
            if (index_caracter == wstring::npos)
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
