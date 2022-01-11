package main

/*
typedef enum option_type {
	option_type_stirng = 's',
	option_type_integer = 'i',
	option_type_bool ='b'
} option_type;

typedef struct command_option {
    char *name;
	int required;
	option_type option_type;
} command_option;

typedef void (*messagecallback)(char *author_username, char *author_mentoin_string, char *channel_id, char *content);

typedef void (*commandhandler)(char *author_username, char *author_mentoin_string, char *channel_id, char *concated_input);

static void discord_onmessage(messagecallback f, char *author_username, char *author_mentoin_id, char *channel_id, char *content)
{
	((messagecallback)f)(author_username, author_mentoin_id, channel_id, content);
}

static void discord_on_command(commandhandler h, char *author_username, char *author_mentoin_string, char *channel_id, char *concated_input)
{
	((commandhandler)h)(author_username, author_mentoin_string, channel_id, concated_input);
}

*/
import "C"
import (
	"fmt"
	"os"
	"strconv"
	"strings"

	"github.com/bwmarrin/discordgo"
)

const (
	maxPendingMsgs = 100

	commandOptionTypeStirng  = 's'
	commandOptionTypeInteger = 'i'
	commandOptionTypeBool    = 'b'
)

type commandOptionType int

type commandOption struct {
	name       string
	required   bool
	optionType commandOptionType
}

type command struct {
	name        string
	description string
	options     []commandOption
	handler     func(s *discordgo.Session, i *discordgo.InteractionCreate)
}

type discordSession struct {
	token           string
	session         *discordgo.Session
	messageCallback C.messagecallback
	channelID       string
	err             error

	commands []*discordgo.ApplicationCommand

	sendMsgs chan message
}

type message struct {
	channelID string
	text      string
}

var (
	defaultSession *discordSession
	commands       []*command
)

func main() {}

//export discord_run
func discord_run(messageCallback C.messagecallback, token *C.char, channelID *C.char) C.int {
	botToken := C.GoString(token)
	channel := C.GoString(channelID)

	defaultSession = newSession(messageCallback, botToken, channel)

	err := defaultSession.open()
	if err != nil {
		return -1
	}

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

//export discord_register_command
func discord_register_command(name *C.char, description *C.char, options []C.command_option, handler C.commandhandler) {
	var commandOptions []commandOption

	for _, opt := range options {
		convertedOption, err := newCommandOption(opt.name, opt.required, opt.option_type)
		if err != nil {
			reportErrorf("%s", err.Error())
			return
		}

		commandOptions = append(commandOptions, convertedOption)
	}

	newCommand(name, description, commandOptions, handler)
}

func newSession(messageCallback C.messagecallback, token string, channelID string) *discordSession {
	return &discordSession{
		token:           token,
		messageCallback: messageCallback,
		sendMsgs:        make(chan message, maxPendingMsgs),
		channelID:       channelID,
	}
}

func (s *discordSession) open() error {
	s.session, s.err = discordgo.New("Bot " + s.token)
	if s.err != nil {
		return s.err
	}

	s.session.Identify.Intents = discordgo.MakeIntent(discordgo.IntentsGuildMessages)

	s.session.AddHandler(func(ses *discordgo.Session, m *discordgo.MessageCreate) {
		if m.Author.ID == ses.State.User.ID || m.ChannelID != s.channelID {
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

func (s *discordSession) stringError() string {
	if s.err != nil {
		return s.err.Error()
	}

	return ""
}

func (s *discordSession) sendMessage(channelID string, text string) {
	select {
	case s.sendMsgs <- message{
		channelID: channelID,
		text:      text,
	}:
	default:
		s.err = fmt.Errorf("can not send message because queue is full")
		reportErrorf(s.err.Error())
	}
}

func (s *discordSession) sendWorker() {
	for msg := range s.sendMsgs {
		_, s.err = s.session.ChannelMessageSend(msg.channelID, msg.text)
		if s.err != nil {
			reportErrorf("can not send message %v", s.err)
		}
	}
}

func newCommandOption(name *C.char, required C.int, optitionType C.option_type) (commandOption, error) {
	var optionType commandOptionType

	switch optitionType {
	case C.option_type_stirng:
		optionType = commandOptionTypeStirng
	case C.option_type_integer:
		optionType = commandOptionTypeInteger
	case C.option_type_bool:
		optionType = commandOptionTypeBool
	default:
		return commandOption{}, fmt.Errorf("unknow option type %v", optitionType)
	}

	return commandOption{
		name:       C.GoString(name),
		required:   int(required) == 1,
		optionType: optionType,
	}, nil
}

func newCommand(name *C.char, description *C.char, options []commandOption, handler C.commandhandler) command {
	return command{
		name:        C.GoString(name),
		description: C.GoString(description),
		options:     options,
		handler: func(s *discordgo.Session, i *discordgo.InteractionCreate) {
			s.InteractionRespond(i.Interaction, &discordgo.InteractionResponse{
				Type: discordgo.InteractionResponseDeferredChannelMessageWithSource,
			})
			C.discord_on_command(handler, C.CString("test"), C.CString("test"), C.CString("123"), commandDataToQuotedCString(options, i.ApplicationCommandData()))
		},
	}
}

func addCommand(command *command) {
	commands = append(commands, command)
}

func reportErrorf(format string, a ...interface{}) {
	fmt.Fprintf(os.Stderr, "discord: "+format+"\n", a)
}

func commandDataToQuotedCString(options []commandOption, data discordgo.ApplicationCommandInteractionData) *C.char {
	var quotedParams []string

	for i, opt := range options {
		var strValue = "\"\""
		interactioninputParam := data.Options[i]

		switch opt.optionType {
		case commandOptionTypeStirng:
			strValue = "\"" + interactioninputParam.StringValue() + "\""
		case commandOptionTypeInteger:
			strValue = "\"" + strconv.Itoa(int(interactioninputParam.IntValue())) + "\""
		case commandOptionTypeBool:
			if interactioninputParam.BoolValue() {
				strValue = "\"1\""
			} else {
				strValue = "\"0\""
			}
		}

		quotedParams = append(quotedParams, strValue)
	}

	return C.CString(strings.Join(quotedParams, " "))
}
