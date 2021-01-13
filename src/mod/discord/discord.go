package main

/*
#include "discord_plugin.h"



static void onmessage(void *f, char *author_username, char *author_mentoin_id, char *channel_id, char *content)
{
	(*(messagecallback)f)(author_username, author_mentoin_id, channel_id, content);
}
*/
import "C"
import (
	"log"
	"unsafe"

	"github.com/bwmarrin/discordgo"
)

const apiVersion = 1

var session *discordgo.Session
var err error
var msgCallback unsafe.Pointer

func main() {}

// export version
func version() C.int {
	return C.int(apiVersion)
}

//export run
func run(messageCallback unsafe.Pointer, token *C.char) C.int {
	msgCallback = messageCallback
	botToken := C.GoString(token)

	session, err = discordgo.New("Bot " + botToken)
	if err != nil {
		return 1
	}

	session.AddHandler(onMessageCreate)
	session.Identify.Intents = discordgo.MakeIntent(discordgo.IntentsGuildMessages)

	var openErrChan chan error

	go func() {
		openErr := session.Open()
		if openErr != nil {
			openErrChan <- openErr
			return
		}

		openErrChan <- nil

		// wait forever
		var wait chan struct{}
		<-wait
	}()

	err = <-openErrChan
	if err != nil {
		return 1
	}

	return 0
}

//export lasterror
func lasterror() *C.char {
	return C.CString(err.Error())
}

//export sendmessage
func sendmessage(channel *C.char, text *C.char) C.int {
	channelID := C.GoString(channel)
	content := C.GoString(text)

	_, err = session.ChannelMessageSend(channelID, content)
	if err != nil {
		return 1
	}

	return 0
}

func onMessageCreate(s *discordgo.Session, m *discordgo.MessageCreate) {
	if m.Author.ID == s.State.User.ID {
		return
	}

	msgContent, replaceErr := m.ContentWithMoreMentionsReplaced(s)
	if replaceErr != nil {
		log.Printf("can not parse message content: %v", replaceErr)
		return
	}

	username := C.CString(m.Author.Username)
	mentoinString := C.CString(m.Author.Mention())
	channelID := C.CString(m.ChannelID)
	content := C.CString(msgContent)

	C.onmessage(msgCallback, username, mentoinString, channelID, content)
}
