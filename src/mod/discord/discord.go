package main

/*
typedef enum {
	option_type_stirng = 's',
	option_type_integer = 'i',
	option_type_bool ='b'
} option_type;

typedef struct {
    char *name;
	int required;
	option_type option_t;
} command_option;

typedef void (*messagecallback)(char *author_username, char *author_mentoin_string, char *channel_id, char *content);

typedef void (*commandhandler)(char *author_username, char *author_mentoin_string, char *channel_id, char *cmd_name, char *concated_input);

static void discord_onmessage(messagecallback f, char *author_username, char *author_mentoin_id, char *channel_id, char *content)
{
	((messagecallback)f)(author_username, author_mentoin_id, channel_id, content);
}

static void discord_on_command(commandhandler h, char *author_username, char *author_mentoin_string, char *channel_id,  char *cmd_name, char *concated_input)
{
	((commandhandler)h)(author_username, author_mentoin_string, channel_id, cmd_name, concated_input);
}

*/
import "C"
import (
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/bwmarrin/discordgo"
)

const (
	maxPendingMsgs = 100

	commandOptionTypeStirng  commandOptionType = 's'
	commandOptionTypeInteger commandOptionType = 'i'
	commandOptionTypeBool    commandOptionType = 'b'
)

type commandOptionType uint8

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

type message struct {
	channelID string
	text      string
}

var (
	defaultSession        *discordSession
	commands              []*command
	commandHandlersByName = map[string]*command{}
)

func main() {}

//export discord_run
func discord_run(messageCallback C.messagecallback, token *C.char, channelID *C.char) C.int {
	botToken := C.GoString(token)
	channel := C.GoString(channelID)

	session := newSession(func(username string, mentoin string, channelID string, content string) {
		cUsername := C.CString(username)
		cMentoinString := C.CString(mentoin)
		cChannelID := C.CString(channelID)
		cContent := C.CString(content)

		C.discord_onmessage(messageCallback, cUsername, cMentoinString, cChannelID, cContent)
	}, botToken, channel)
	defaultSession = session

	err := session.init()
	if err != nil {
		return -1
	}

	session.registerHandlers()

	err = session.open()
	if err != nil {
		return -1
	}

	time.Sleep(time.Second * 2)

	session.registerCommands()

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
		convertedOption, err := newCommandOption(opt.name, opt.required, opt.option_t)
		if err != nil {
			reportErrorf("%s", err.Error())
			return
		}

		commandOptions = append(commandOptions, convertedOption)
	}

	command := newCommand(name, description, commandOptions, handler)

	fmt.Printf("%#v", command)

	addCommand(command)
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

func newCommand(name *C.char, description *C.char, options []commandOption, handler C.commandhandler) *command {
	return &command{
		name:        C.GoString(name),
		description: C.GoString(description),
		options:     options,
		handler: func(s *discordgo.Session, i *discordgo.InteractionCreate) {

			err := s.InteractionRespond(i.Interaction, &discordgo.InteractionResponse{
				Type: discordgo.InteractionResponseChannelMessageWithSource,
				Data: &discordgo.InteractionResponseData{
					Content: commandWithParamsEcho(C.GoString(name), i.ApplicationCommandData()),
				},
			})
			if err != nil {
				reportErrorf("cannot make response to discord interaction: %v", err)
			}

			author := ""
			mentoin := ""

			if i.User != nil {
				author = i.User.Username
				mentoin = i.User.Mention()
			} else {
				author = i.Member.User.Username
				mentoin = i.Member.User.Mention()
			}

			C.discord_on_command(handler, C.CString(author), C.CString(mentoin), C.CString(i.ChannelID), name, commandDataToQuotedCString(options, i.ApplicationCommandData()))
		},
	}
}

func addCommand(command *command) {
	commands = append(commands, command)
	commandHandlersByName[command.name] = command
}

func reportErrorf(format string, a ...interface{}) {
	fmt.Fprintf(os.Stderr, "discord: "+format+"\n", a)
}

// commandDataToQuotedCString convert discord slash command arguments to single string with double quoted separated by space
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

func internalCommandToDiscordCommand(command *command) *discordgo.ApplicationCommand {
	var options []*discordgo.ApplicationCommandOption

	for _, cmdOption := range command.options {
		options = append(options, &discordgo.ApplicationCommandOption{
			Type:        internalToDiscordOptionType(cmdOption.optionType),
			Name:        cmdOption.name,
			Description: cmdOption.name,
			Required:    cmdOption.required,
		})
	}

	return &discordgo.ApplicationCommand{
		Name:        command.name,
		Description: command.description,
		Options:     options,
	}
}

func internalToDiscordOptionType(optionType commandOptionType) discordgo.ApplicationCommandOptionType {
	switch optionType {
	case commandOptionTypeStirng:
		return discordgo.ApplicationCommandOptionString
	case commandOptionTypeInteger:
		return discordgo.ApplicationCommandOptionInteger
	case commandOptionTypeBool:
		return discordgo.ApplicationCommandOptionBoolean
	default:
		reportErrorf("unknown option type %v", optionType)
		panic("unknown option type")
	}
}

func commandWithParamsEcho(cmd string, data discordgo.ApplicationCommandInteractionData) string {
	commandWithParams := cmd

	for _, v := range data.Options {
		commandWithParams += fmt.Sprintf(" %v", v.Value)
	}

	return commandWithParams
}
