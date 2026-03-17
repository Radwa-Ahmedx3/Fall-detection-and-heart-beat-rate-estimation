/*
 * telegram.h
 *
 * Created: 16/03/2026 08:30:09
 *  Author: maria
 */ 


#ifndef TELEGRAM_H_
#define TELEGRAM_H_

/* Telegram bot credentials */
#define TELEGRAM_BOT_TOKEN "8224115321:AAE3T13qW805CE85Kw_zlYL66ruF9M26FsI"
#define TELEGRAM_CHAT_ID   "6243078279"

/* HAL functions for Telegram */
void Telegram_Init(void);
void Telegram_SendMessage(char *message);
void Telegram_SendEmergency(int heartRate);
void Telegram_CheckCommands(void);

#endif /* TELEGRAM_H_ */