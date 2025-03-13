#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>
#pragma pack(1)
typedef struct {
    uint32_t pgn;          // Parameter Group Number
    uint8_t data[8];       // J1939 data (max 8 tavua)
    uint8_t priority;      // Prioriteetti
    uint32_t sourceAddr;   // Lähettäjän osoite
} J1939_Message_t;

typedef struct {
    char disp_name[20];
    int pituus;
    int korjaus;
} kauha_t;

typedef struct {
    int pituus;
    int korjaus;
} puomi_t;

typedef struct {
    J1939_Message_t name_id;
    J1939_Message_t last_kulma_frame;
    int16_t last_kulma;
    int position;
    int last_update;
} kulma_anturi_t;

typedef struct {
    int version;
    kulma_anturi_t kulma_anturit[4];
    puomi_t puomit[3];
    kauha_t kauhat[5];
    uint8_t current_kauha;
    int korkeus;
    int korkeus_ilman_kaatoa;
    int kaato;
    int tila;                      // flagejä käyttäen määritetään systeemin tila
} system_data_t;

// PAKETIT TIETOKONEELLE
#define ID_J1939_MSG (0x11)
#define ID_SYSTEM_DATA_PACKET (0x22)
#define ID_ERROR_MSG (0x33)
#define ID_INFO_MSG (0x44)
typedef struct {
    uint8_t start;
    uint8_t id;
    J1939_Message_t frame;
    uint8_t end;
} can_msg_to_pc;

typedef struct {
    uint8_t start;
    uint8_t id;
    system_data_t data;
    uint8_t end;
} system_data_to_pc;
typedef struct{
    uint8_t start;
    uint8_t id;
    char msg[64];
    uint8_t end;
}msg_packet_t;

// KOMENTOPAKETTI
typedef struct
{
    uint8_t start;
    char msg[45];
    uint8_t end;
}cmd_packet_t;
#pragma pack()

#endif // TYPES_H
