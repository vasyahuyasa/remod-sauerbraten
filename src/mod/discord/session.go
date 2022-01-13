package main

import (
	"fmt"
	"log"

	"github.com/bwmarrin/discordgo"
)

type messageCallback func(username string, mentoin string, channelID string, content string)

type discordSession struct {
	token           string
	session         *discordgo.Session
	messageCallback messageCallback
	channelID       string
	err             error

	sendMsgs chan message
}

func newSession(messageCallback messageCallback, token string, channelID string) *discordSession {
	return &discordSession{
		token:           token,
		messageCallback: messageCallback,
		sendMsgs:        make(chan message, maxPendingMsgs),
		channelID:       channelID,
	}
}

func (s *discordSession) init() error {
	s.session, s.err = discordgo.New("Bot " + s.token)
	if s.err != nil {
		return s.err
	}

	s.session.Identify.Intents = discordgo.MakeIntent(discordgo.IntentsGuildMessages)

	return nil
}

func (s *discordSession) registerHandlers() {
	s.session.AddHandler(func(ses *discordgo.Session, m *discordgo.MessageCreate) {
		if m.Author.ID == ses.State.User.ID || m.ChannelID != s.channelID {
			return
		}

		s.messageCallback(m.Author.Username, m.Author.Mention(), m.ChannelID, m.ContentWithMentionsReplaced())
	})

	s.session.AddHandler(func(s *discordgo.Session, r *discordgo.Ready) {
		log.Println("Bot is up!")
	})

	for _, command := range commands {
		s.session.AddHandler(func(s *discordgo.Session, i *discordgo.InteractionCreate) {
			if cmd, ok := commandHandlersByName[command.name]; ok {
				cmd.handler(s, i)
			}
		})
	}
}

func (s *discordSession) registerCommands() {
	for _, command := range commands {
		discordCommand := internalCommandToDiscordCommand(command)
		_, err := s.session.ApplicationCommandCreate(s.session.State.User.ID, "", discordCommand)
		if err != nil {
			reportErrorf("cannot register '%s' command: %v", command.name, err)
		} else {
			reportErrorf("command '%s' registerd", command.name)
		}
	}
}

func (s *discordSession) open() error {
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
