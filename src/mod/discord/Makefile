default: discord.so

clean:
	rm discord.so

discord: discord.so

discord.so:
	CGO_ENABLED=1 go build -o discord.so
