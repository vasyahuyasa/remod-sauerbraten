package main

/*
typedef void *messagecallback(char *author_username, char *author_mentoin_id, char *channel_id, char *content)

void onmessage(void *f, char *author_username, char *author_mentoin_id, char *channel_id, char *content)
{
	(messagecallback*)f(author_username, author_mentoin_id, channel_id, content);
}
*/
import "C"

func main() {}

//export start
func run(token string) int {
	token = token

	return 0
}

func onMessage() {

}
