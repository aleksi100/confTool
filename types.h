/*
 * types.h
 *
 *  Created on: Mar 11, 2025
 *      Author: Aleksi Eskola
 */

#ifndef INC_TYPES_H_
#define INC_TYPES_H_
#include "stdint.h"
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
}kauha_t;

typedef struct {
    int pituus;
    int korjaus;
}puomi_t;

typedef struct {
    uint32_t id;
    uint32_t sourceAddr;
    int16_t last_kulma;
    uint8_t position;
    int last_update;
}kulma_anturi_t;


/*
 * Yleiskattava tietopaketti systeemiin tilasta.
 * start, end ja packet_id kentät ovat paketin lähettämistä varten.
 * Tämä alustetaan main.c ja käytetään kaikkialla.
 * Tarittaessa mukkaamiseen käytetään freertos lukko-ominaisuuksia,
 * jos tietoa muutetaan useassa paikassa.*/
typedef struct {
    uint8_t start;
    uint8_t packet_id;
    int version;
    kulma_anturi_t kulma_anturit[4];
    puomi_t puomit[3];
    kauha_t kauhat[5];
    uint8_t current_kauha;
    int korkeus;
    int korkeus_ilman_kaatoa;
    int kaato;
    int tila;// flagejä käyttäen määritetään systetemin tila
    uint8_t end;
} system_data_t;

// PAKETIT TIETOKONEELLE
#define ID_J1939_MSG (0x11)
#define ID_SYSTEM_DATA_PACKET (0x22)
#define ID_ERROR_MSG (0x33)
#define ID_INFO_MSG (0x44)
#define ID_COMMAND_PACKET (0x55)
typedef struct{
    uint8_t start;
    uint8_t id;
    J1939_Message_t frame;
    uint8_t end;
}can_msg_to_pc;


// käytetään perus viesteihin
typedef struct{
    uint8_t start;
    uint8_t id;
    char msg[64];
    uint8_t end;
}msg_packet_t;



#pragma pack()
#endif /* INC_TYPES_H_ */
