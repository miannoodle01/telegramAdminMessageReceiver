#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <string>
#include <exception>
#include <tgbot/tgbot.h>
using namespace std;
using namespace TgBot;

void signalHandler(int i);

int main() {
    string token = "TOKEN";
    string startMessage = "welcome to bot";
    string welcomeAdmin = "welcome admin";
    string deliveredMessage = "admin will receive your message";
    string enterMessage = "enter message to be send";
    string messageNameDesc = "user name is: ";
    string messageBuffer;
    string textBuffer;
    int64_t adminChatId = 0; // Put your chat id here instead of zero.
    int64_t destChatIdBuffer;
    Bot bot(token);
    
    // Signal handling part
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    // End of signal handling part

    // Inline keyboard part
    InlineKeyboardMarkup::Ptr answerKeyboard(new InlineKeyboardMarkup);
    vector<InlineKeyboardButton::Ptr> row0;
    InlineKeyboardButton::Ptr answerButton(new InlineKeyboardButton);
    answerButton->text = "answer";
    answerButton->callbackData = "answer";
    row0.push_back(answerButton);
    answerKeyboard->inlineKeyboard.push_back(row0);
    // End of inline keyboard part

    bot.getEvents().onCommand("start", [
        &bot,
        &startMessage,
        &adminChatId,
        &welcomeAdmin
    ] (Message::Ptr message) {
        printf("chatId is: %" PRId64 "\n", message->chat->id);
        if(message->chat->id == adminChatId) {
            bot.getApi().sendMessage(message->chat->id, welcomeAdmin);
        } else {
            bot.getApi().sendMessage(message->chat->id, startMessage);
        }
        
    });

    bot.getEvents().onNonCommandMessage([
        &bot,
        &adminChatId,
        &deliveredMessage,
        &answerKeyboard,
        &destChatIdBuffer,
        &messageBuffer,
        &messageNameDesc
    ] (Message::Ptr message) {
        printf("Written message is: %s\n", message->text.c_str());
        if (message->chat->id != adminChatId) {
            destChatIdBuffer = message->chat->id;
            messageBuffer = messageNameDesc +
                message->chat->firstName + " " +
                message->chat->lastName + "\n" +
                message->text;
            bot.getApi().sendMessage(
                adminChatId,
                messageBuffer,
                false,
                0,
                answerKeyboard
            );
            bot.getApi().sendMessage(
                message->chat->id,
                deliveredMessage
            );
        }
    });

    bot.getEvents().onCallbackQuery([
        &bot,
        &answerKeyboard,
        &adminChatId,
        &destChatIdBuffer,
        &enterMessage,
        &messageBuffer
    ] (CallbackQuery::Ptr query) {
        if(StringTools::startsWith(query->data, "answer")) {
            bot.getApi().sendMessage(adminChatId, enterMessage);
            bot.getEvents().onAnyMessage([&] (Message::Ptr message) {
                bot.getApi().sendMessage(destChatIdBuffer, message->text);
            });
        }
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgLongPoll longPoll(bot);
        while(1) {
            printf("long poll started.\n");
            longPoll.start();
            bot.getApi().deleteWebhook();
        }
    } catch (exception &e) {
        /* Uncomment line below if you want to clear terminal on error. */
        //system("clear");
        printf(
            "long poll throwed error, error is: %s\nrestarting poll in...\n",
            e.what()
        );
        for (int i = 3; i >= 0; i--) {
            sleep(1);
            printf("%i\n", i);
        }
        main();
    }
    return 0;
}

void signalHandler(int i) {
    printf("\nCtrl + C pressed. Exiting...\n");
    exit(0);
}
