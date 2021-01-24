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
	"log"

	"github.com/bwmarrin/discordgo"
)

const maxPendingMsgs = 100

type discordSession struct {
	token           string
	session         *discordgo.Session
	messageCallback C.messagecallback
	err             error

	channels []string
	msgs     chan message
}

type message struct {
	channelID string
	text      string
}

var defaultSession *discordSession

func main() {}

//export discord_run
func discord_run(messageCallback C.messagecallback, token *C.char) C.int {
	botToken := C.GoString(token)

	defaultSession = newSession(messageCallback, botToken)

	err := defaultSession.open()
	if err != nil {
		return -1
	}

	log.Println(defaultSession.channels)

	return 0
}

//export discord_lasterror
func discord_lasterror() *C.char {
	return C.CString(defaultSession.stringError())
}

//export discord_sendmessage
func discord_sendmessage(channel *C.char, text *C.char) {
	channelID := C.GoString(channel)
	content := C.GoString(text)

	defaultSession.sendMessage(channelID, content)
}

func newSession(messageCallback C.messagecallback, token string) *discordSession {
	return &discordSession{
		token:           token,
		messageCallback: messageCallback,
		msgs:            make(chan message, maxPendingMsgs),
	}
}

func (s *discordSession) open() error {
	s.session, s.err = discordgo.New("Bot " + s.token)
	if s.err != nil {
		return s.err
	}

	s.session.Identify.Intents = discordgo.MakeIntent(discordgo.IntentsGuildMessages)

	s.session.AddHandler(func(ses *discordgo.Session, m *discordgo.MessageCreate) {
		if m.Author.ID == ses.State.User.ID {
			return
		}

		username := C.CString(m.Author.Username)
		mentoinString := C.CString(m.Author.Mention())
		channelID := C.CString(m.ChannelID)
		content := C.CString(m.ContentWithMentionsReplaced())

		C.discord_onmessage(s.messageCallback, username, mentoinString, channelID, content)
	})

	// TODO: session.Client with timeout

	s.err = s.session.Open()
	if s.err != nil {
		return s.err
	}

	go s.sendWorker()

	return nil
}

func (s *discordSession) lookupChannels() error {
	guilds, err := s.session.UserGuilds(0, "", "")
	if err != nil {
		return err
	}

	for _, guild := range guilds {
		channels, err := s.session.GuildChannels(guild.ID)
		if err != nil {
			return err
		}

		for _, channel := range channels {
			s.channels = append(s.channels, channel.ID)
		}
	}

	log.Println(guilds)

	return nil
}

func (s *discordSession) stringError() string {
	if s.err != nil {
		return s.err.Error()
	}

	return ""
}

func (s *discordSession) sendMessage(channelID string, text string) {
	s.msgs <- message{
		channelID: channelID,
		text:      text,
	}
}

func (s *discordSession) sendWorker() {
	for msg := range s.msgs {
		_, s.err = s.session.ChannelMessageSend(msg.channelID, msg.text)
	}
}
