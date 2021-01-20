package main

/*
typedef void (*messagecallback)(char *author_username, char *author_mentoin_string, char *channel_id, char *content);

static void discord_onmessage(messagecallback f, char *author_username, char *author_mentoin_id, char *channel_id, char *content)
{
	((messagecallback)f)(author_username, author_mentoin_id, channel_id, content);
}
*/
import "C"
import (
	"errors"

	"github.com/bwmarrin/discordgo"
)

var session *discordgo.Session
var err error

func main() {}

//export discord_run
func discord_run(messageCallback C.messagecallback, token *C.char) C.int {
	botToken := C.GoString(token)

	session, err = openSession(messageCallback, botToken)
	if err != nil {
		return 1
	}

	return 0
}

//export discord_lasterror
func discord_lasterror() *C.char {
	return C.CString(lastErrorString())
}

//export discord_sendmessage
func discord_sendmessage(channel *C.char, text *C.char) C.int {
	channelID := C.GoString(channel)
	content := C.GoString(text)

	err = sendMessage(channelID, content)
	if err != nil {
		return 1
	}

	return 0
}

func openSession(messageCallback C.messagecallback, token string) (*discordgo.Session, error) {
	session, err = discordgo.New("Bot " + token)
	if err != nil {
		return nil, err
	}

	session.Identify.Intents = discordgo.MakeIntent(discordgo.IntentsGuildMessages)

	session.AddHandler(func(s *discordgo.Session, m *discordgo.MessageCreate) {
		if m.Author.ID == s.State.User.ID {
			return
		}

		username := C.CString(m.Author.Username)
		mentoinString := C.CString(m.Author.Mention())
		channelID := C.CString(m.ChannelID)
		content := C.CString(m.ContentWithMentionsReplaced())

		C.discord_onmessage(messageCallback, username, mentoinString, channelID, content)
	})

	err = session.Open()
	if err != nil {
		return nil, err
	}

	// TODO: session.Client with timeout

	return session, nil
}

func lastErrorString() string {
	if err != nil {
		return err.Error()
	}

	return ""
}

func sendMessage(channelID string, text string) error {
	if session == nil {
		return errors.New("discord does not running")
	}

	// TODO: use buffered channel for save latency because C -> Go call takes 1-5 ms/op

	_, err = session.ChannelMessageSend(channelID, text)

	return err
}
