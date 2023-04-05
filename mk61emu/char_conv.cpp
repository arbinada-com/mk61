#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "char_conv.h"

typedef struct utf8_to_win1251_struct
{
    int code_utf8;
    unsigned char code_windows1251;
} utf8_to_win1251;

const utf8_to_win1251 table_utf8_to_win1251[127] =
{
    {1027, 129},
    {8225, 135},
    {1046, 198},
    {8222, 132},
    {1047, 199},
    {1168, 165},
    {1048, 200},
    {1113, 154},
    {1049, 201},
    {1045, 197},
    {1050, 202},
    {1028, 170},
    { 160, 160},
    {1040, 192},
    {1051, 203},
    { 164, 164},
    { 166, 166},
    { 167, 167},
    { 169, 169},
    { 171, 171},
    { 172, 172},
    { 173, 173},
    { 174, 174},
    {1053, 205},
    { 176, 176},
    { 177, 177},
    {1114, 156},
    { 181, 181},
    { 182, 182},
    { 183, 183},
    {8221, 148},
    { 187, 187},
    {1029, 189},
    {1056, 208},
    {1057, 209},
    {1058, 210},
    {8364, 136},
    {1112, 188},
    {1115, 158},
    {1059, 211},
    {1060, 212},
    {1030, 178},
    {1061, 213},
    {1062, 214},
    {1063, 215},
    {1116, 157},
    {1064, 216},
    {1065, 217},
    {1031, 175},
    {1066, 218},
    {1067, 219},
    {1068, 220},
    {1069, 221},
    {1070, 222},
    {1032, 163},
    {8226, 149},
    {1071, 223},
    {1072, 224},
    {8482, 153},
    {1073, 225},
    {8240, 137},
    {1118, 162},
    {1074, 226},
    {1110, 179},
    {8230, 133},
    {1075, 227},
    {1033, 138},
    {1076, 228},
    {1077, 229},
    {8211, 150},
    {1078, 230},
    {1119, 159},
    {1079, 231},
    {1042, 194},
    {1080, 232},
    {1034, 140},
    {1025, 168},
    {1081, 233},
    {1082, 234},
    {8212, 151},
    {1083, 235},
    {1169, 180},
    {1084, 236},
    {1052, 204},
    {1085, 237},
    {1035, 142},
    {1086, 238},
    {1087, 239},
    {1088, 240},
    {1089, 241},
    {1090, 242},
    {1036, 141},
    {1041, 193},
    {1091, 243},
    {1092, 244},
    {8224, 134},
    {1093, 245},
    {8470, 185},
    {1094, 246},
    {1054, 206},
    {1095, 247},
    {1096, 248},
    {8249, 139},
    {1097, 249},
    {1098, 250},
    {1044, 196},
    {1099, 251},
    {1111, 191},
    {1055, 207},
    {1100, 252},
    {1038, 161},
    {8220, 147},
    {1101, 253},
    {8250, 155},
    {1102, 254},
    {8216, 145},
    {1103, 255},
    {1043, 195},
    {1105, 184},
    {1039, 143},
    {1026, 128},
    {1106, 144},
    {8218, 130},
    {1107, 131},
    {8217, 146},
    {1108, 186},
    {1109, 190}
};

char convert_char_utf8_to_windows1251(const int code_utf8)
{
    char result = '?';
    if (code_utf8 < 128)
        result = code_utf8;
    else
    {
        int i = 0;
        for (i = 0; i < 128; i++)
        {
            if (table_utf8_to_win1251[i].code_utf8 == code_utf8)
            {
                result = table_utf8_to_win1251[i].code_windows1251;
                break;
            }
        }
    }
    return result;
}

// See detailed table
// http://www.lingua-systems.com/knowledge/unicode-mappings/windows-1251-to-unicode.html

typedef struct utf8_code_struct
{
    uint8_t c1;
    uint8_t c2;
    uint8_t c3;
} utf8_code;

const utf8_code table_win1251_to_utf8[] = // Starting from 128th symbol (0x80)
{
    {0xD0, 0x82, 0x00}, // 128 - Cyrillic Capital Letter Dje
    {0xD0, 0x83, 0x00}, // 129 - Cyrillic Capital Letter Gje
    {0xE2, 0x80, 0x9A}, // 130 - Single Low-9 Quotation Mark
    {0xD1, 0x93, 0x00}, // Cyrillic Small Letter Gje
    {0xE2, 0x80, 0x9E}, // Double Low-9 Quotation Mark
    {0xE2, 0x80, 0xA6}, // Horizontal Ellipsis
    {0xE2, 0x80, 0xA0}, // Dagger
    {0xE2, 0x80, 0xA1}, // Double Dagger
    {0xE2, 0x82, 0xAC}, // Euro Sign
    {0xE2, 0x80, 0xB0}, // Per Mille Sign
    {0xD0, 0x89, 0x00}, // Cyrillic Capital Letter Lje
    {0xE2, 0x80, 0xB9}, // Single Left-pointing Angle Quotation Mark
    {0xD0, 0x8A, 0x00}, // 140 - Cyrillic Capital Letter Nje
    {0xD0, 0x8C, 0x00}, // Cyrillic Capital Letter Kje
    {0xD0, 0x8B, 0x00}, // Cyrillic Capital Letter Tshe
    {0xD0, 0x8F, 0x00}, // Cyrillic Capital Letter Dzhe
    {0xD1, 0x92, 0x00}, // Cyrillic Small Letter Dje
    {0xE2, 0x80, 0x98}, // Left Single Quotation Mark
    {0xE2, 0x80, 0x99}, // Right Single Quotation Mark
    {0xE2, 0x80, 0x9C}, // Left Double Quotation Mark
    {0xE2, 0x80, 0x9D}, // Right Double Quotation Mark
    {0xE2, 0x80, 0xA2}, // Bullet
    {0xE2, 0x80, 0x93}, // 150 - En Dash
    {0xE2, 0x80, 0x94}, // Em Dash
    {0x00, 0x00, 0x00}, // ??
    {0xE2, 0x84, 0xA2}, // Trade Mark Sign
    {0xD1, 0x99, 0x00}, // Cyrillic Small Letter Lje
    {0xE2, 0x80, 0xBA}, // Single Right-pointing Angle Quotation Mark
    {0xD1, 0x9A, 0x00}, // Cyrillic Small Letter Nje
    {0xD1, 0x9C, 0x00}, // Cyrillic Small Letter Kje
    {0xD1, 0x9B, 0x00}, // Cyrillic Small Letter Tshe
    {0xD1, 0x9F, 0x00}, // Cyrillic Small Letter Dzhe
    {0xC2, 0xA0, 0x00}, // 160 - No-break Space
    {0xD0, 0x8E, 0x00}, // Cyrillic Capital Letter Short U
    {0xD1, 0x9E, 0x00}, // Cyrillic Small Letter Short U
    {0xD0, 0x88, 0x00}, // Cyrillic Capital Letter Je
    {0xC2, 0xA4, 0x00}, // Currency Sign
    {0xD2, 0x90, 0x00}, // Cyrillic Capital Letter Ghe With Upturn
    {0xC2, 0xA6, 0x00}, // Broken Bar
    {0xC2, 0xA7, 0x00}, // Section Sign
    {0xD0, 0x81, 0x00}, // Cyrillic Capital Letter Io
    {0xC2, 0xA9, 0x00}, // Copyright Sign
    {0xD0, 0x84, 0x00}, // 170 - Cyrillic Capital Letter Ukrainian Ie
    {0xC2, 0xAB, 0x00}, // Left-pointing Double Angle Quotation Mark
    {0xC2, 0xAC, 0x00}, // Not Sign
    {0xC2, 0xAD, 0x00}, // Soft Hyphen
    {0xC2, 0xAE, 0x00}, // Registered Sign
    {0xD0, 0x87, 0x00}, // Cyrillic Capital Letter Yi
    {0xC2, 0xB0, 0x00}, // Degree Sign
    {0xC2, 0xB1, 0x00}, // Plus-minus Sign
    {0xD0, 0x86, 0x00}, // Cyrillic Capital Letter Byelorussian-ukrainian I
    {0xD1, 0x96, 0x00}, // Cyrillic Small Letter Byelorussian-ukrainian I
    {0xD2, 0x91, 0x00}, // 180 - Cyrillic Small Letter Ghe With Upturn
    {0xC2, 0xB5, 0x00}, // Micro Sign
    {0xC2, 0xB6, 0x00}, // Pilcrow Sign
    {0xC2, 0xB7, 0x00}, // Middle Dot
    {0xD1, 0x91, 0x00}, // Cyrillic Small Letter Io
    {0xE2, 0x84, 0x96}, // Numero Sign
    {0xD1, 0x94, 0x00}, // Cyrillic Small Letter Ukrainian Ie
    {0xC2, 0xBB, 0x00}, // Right-pointing Double Angle Quotation Mark
    {0xD1, 0x98, 0x00}, // Cyrillic Small Letter Je
    {0xD0, 0x85, 0x00}, // Cyrillic Capital Letter Dze
    {0xD1, 0x95, 0x00}, // 190 - Cyrillic Small Letter Dze
    {0xD1, 0x97, 0x00}, // Cyrillic Small Letter Yi
    {0xD0, 0x90, 0x00}, // Cyrillic Capital Letter A
    {0xD0, 0x91, 0x00}, // Cyrillic Capital Letter Be
    {0xD0, 0x92, 0x00}, // Capital Letter Ve
    {0xD0, 0x93, 0x00}, // Capital Letter Ghe
    {0xD0, 0x94, 0x00}, // Capital Letter De
    {0xD0, 0x95, 0x00}, // Capital Letter Ie
    {0xD0, 0x96, 0x00}, // Capital Letter Zhe
    {0xD0, 0x97, 0x00}, // Capital Letter Ze
    {0xD0, 0x98, 0x00}, // 200 - Capital Letter I
    {0xD0, 0x99, 0x00}, // Capital Letter Short I
    {0xD0, 0x9A, 0x00}, // Capital Letter Ka
    {0xD0, 0x9B, 0x00}, // Capital Letter El
    {0xD0, 0x9C, 0x00}, // Capital Letter Em
    {0xD0, 0x9D, 0x00}, // Capital Letter En
    {0xD0, 0x9E, 0x00}, // Capital Letter O
    {0xD0, 0x9F, 0x00}, // Capital Letter Pe
    {0xD0, 0xA0, 0x00}, // Capital Letter Er
    {0xD0, 0xA1, 0x00}, // Capital Letter Es
    {0xD0, 0xA2, 0x00}, // 210 - Capital Letter Te
    {0xD0, 0xA3, 0x00}, // Capital Letter U
    {0xD0, 0xA4, 0x00}, // Capital Letter Ef
    {0xD0, 0xA5, 0x00}, // Capital Letter Ha
    {0xD0, 0xA6, 0x00}, // Capital Letter Tse
    {0xD0, 0xA7, 0x00}, // Capital Letter Che
    {0xD0, 0xA8, 0x00}, // Capital Letter Sha
    {0xD0, 0xA9, 0x00}, // Capital Letter Shcha
    {0xD0, 0xAA, 0x00}, // Capital Letter Hard Sign
    {0xD0, 0xAB, 0x00}, // Capital Letter Yeru
    {0xD0, 0xAC, 0x00}, // 220 - Capital Letter Soft Sign
    {0xD0, 0xAD, 0x00}, // Capital Letter E
    {0xD0, 0xAE, 0x00}, // Capital Letter Yu
    {0xD0, 0xAF, 0x00}, // Capital Letter Ya
    {0xD0, 0xB0, 0x00}, // Small Letter A
    {0xD0, 0xB1, 0x00}, // Small Letter Be
    {0xD0, 0xB2, 0x00}, // Small Letter Ve
    {0xD0, 0xB3, 0x00}, // Small Letter Ghe
    {0xD0, 0xB4, 0x00}, // Small Letter De
    {0xD0, 0xB5, 0x00}, // Small Letter Ie
    {0xD0, 0xB6, 0x00}, // 230 - Small Letter Zhe
    {0xD0, 0xB7, 0x00}, // Small Letter Ze
    {0xD0, 0xB8, 0x00}, // Small Letter I
    {0xD0, 0xB9, 0x00}, // Small Letter Short I
    {0xD0, 0xBA, 0x00}, // Small Letter Ka
    {0xD0, 0xBB, 0x00}, // Small Letter El
    {0xD0, 0xBC, 0x00}, // Small Letter Em
    {0xD0, 0xBD, 0x00}, // Small Letter En
    {0xD0, 0xBE, 0x00}, // Small Letter O
    {0xD0, 0xBF, 0x00}, // Small Letter Pe
    {0xD1, 0x80, 0x00}, // 240 - Small Letter Er
    {0xD1, 0x81, 0x00}, // Small Letter Es
    {0xD1, 0x82, 0x00}, // Small Letter Te
    {0xD1, 0x83, 0x00}, // Small Letter U
    {0xD1, 0x84, 0x00}, // Small Letter Ef
    {0xD1, 0x85, 0x00}, // Small Letter Ha
    {0xD1, 0x86, 0x00}, // Small Letter Tse
    {0xD1, 0x87, 0x00}, // Small Letter Che
    {0xD1, 0x88, 0x00}, // Small Letter Sha
    {0xD1, 0x89, 0x00}, // Small Letter Shcha
    {0xD1, 0x8A, 0x00}, // 250 - Small Letter Hard Sign
    {0xD1, 0x8B, 0x00}, // Small Letter Yeru
    {0xD1, 0x8C, 0x00}, // Small Letter Soft Sign
    {0xD1, 0x8D, 0x00}, // Small Letter E
    {0xD1, 0x8E, 0x00}, // Small Letter Yu
    {0xD1, 0x8F, 0x00}  // 256 - Small Letter Ya
};


int convert_utf8_to_windows1251(const char* utf8, char* windows1251, const size_t max_len)
{
    size_t len1 = strlen(utf8);
    size_t i = 0;
    size_t j = 0;
    while (i < len1 && j < max_len)
    {
        uint8_t curr_code = (uint8_t)utf8[i];
        if (curr_code < 128)
        {
            windows1251[j++] = curr_code;
            i++;
        }
        else
        {
            int found = 0;
            int k = 0;
            while (k < 128)
            {
                if (table_win1251_to_utf8[k].c1 == curr_code)
                {
                    if (i + 1 < len1)
                    {
                        uint8_t next_code = (uint8_t)utf8[i + 1];
                        if (table_win1251_to_utf8[k].c2 == next_code)
                        {
                            if (table_win1251_to_utf8[k].c3 == 0)
                            {
                                found = 1;
                                i++;
                            }
                            else
                            {
                                if (i + 2 < len1)
                                {
                                    next_code = (uint8_t)utf8[i + 2];
                                    if (table_win1251_to_utf8[k].c3 == next_code)
                                    {
                                        found = 1;
                                        i += 2;
                                    }
                                }
                            }
                        }
                    }
                }
                if (found == 1)
                {
                    windows1251[j++] = k + 128;
                    break;
                }
                k++;
            }
            if (found == 0)
                return 0;
            i++;
        }
    }
    windows1251[j] = 0;
    return j;
}

int convert_windows1251_to_utf8(const char* windows1251, char* utf8, size_t max_len)
{
    size_t len1 = strlen(windows1251);
    size_t i = 0;
    size_t j = 0;
    for (; i < len1 && j < max_len; i++)
    {
        uint8_t curr_code = (uint8_t)windows1251[i];
        if (curr_code < 128) // 0x80
        {
            utf8[j++] = windows1251[i];
        }
        else
        {
            utf8[j++] = table_win1251_to_utf8[curr_code - 128].c1;
            utf8[j++] = table_win1251_to_utf8[curr_code - 128].c2;
            if (table_win1251_to_utf8[curr_code - 128].c3 != 0)
                utf8[j++] = table_win1251_to_utf8[curr_code - 128].c3;
        }
    }
    utf8[j] = 0;
    return j;
}

int transliterate_from_windows1251(const char *src, char *dest, const size_t max_len)
{
    size_t src_lenght = strlen(src);
    size_t i = 0;
    size_t j = 0;
    for (i = 0; i < src_lenght && j < max_len; i++)
    {
        uint8_t curr_code = (uint8_t)src[i];
        if (curr_code < 128)
        {
            dest[j++] = src[i];
        }
        else
        {
            switch(curr_code)
            {
            case L_A:
                dest[j++] = 'A';
                break;
            case L_B:
                dest[j++] = 'B';
                break;
            case L_V:
                dest[j++] = 'V';
                break;
            case L_G:
                dest[j++] = 'G';
                break;
            case L_D:
                dest[j++] = 'D';
                break;
            case L_E:
                dest[j++] = 'E';
                break;
            case L_ZH:
                dest[j++] = 'Z';
                if (j < max_len)
                    dest[j++] = 'H';
                break;
            case L_Z:
                dest[j++] = 'Z';
                break;
            case L_I:
                dest[j++] = 'I';
                break;
            case L_J:
                dest[j++] = 'J';
                break;
            case L_K:
                dest[j++] = 'K';
                break;
            case L_L:
                dest[j++] = 'L';
                break;
            case L_M:
                dest[j++] = 'M';
                break;
            case L_N:
                dest[j++] = 'N';
                break;
            case L_O:
                dest[j++] = 'O';
                break;
            case L_P:
                dest[j++] = 'P';
                break;
            case L_R:
                dest[j++] = 'R';
                break;
            case L_S:
                dest[j++] = 'S';
                break;
            case L_T:
                dest[j++] = 'T';
                break;
            case L_U:
                dest[j++] = 'U';
                break;
            case L_F:
                dest[j++] = 'F';
                break;
            case L_H:
                dest[j++] = 'H';
                break;
            case L_C:
                dest[j++] = 'C';
                break;
            case L_CH:
                dest[j++] = 'C';
                if (j < max_len)
                    dest[j++] = 'H';
                break;
            case L_SH:
                dest[j++] = 'S';
                if (j < max_len)
                    dest[j++] = 'H';
                break;
            case L_SCH:
                dest[j++] = 'S';
                if (j < max_len)
                    dest[j++] = 'C';
                if (j < max_len)
                    dest[j++] = 'H';
                break;
            case L_TZ:
                dest[j++] = '\'';
                break;
            case L_Y:
                dest[j++] = 'Y';
                break;
            case L_MZ:
                dest[j++] = '\'';
                break;
            case L_EH:
                dest[j++] = 'E';
                break;
            case L_JU:
                dest[j++] = 'J';
                if (j < max_len)
                    dest[j++] = 'U';
                break;
            case L_JA:
                dest[j++] = 'J';
                if (j < max_len)
                    dest[j++] = 'A';
                break;
            default:
                dest[j++] = curr_code;
            }
        }
    }
    dest[j] = 0;
    return j;
}
