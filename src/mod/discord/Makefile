default: libdiscord.a

clean:
	rm -r libdiscord.a libdiscord.h

discord: libdiscord.a

libdiscord.a:
	CGO_ENABLED=1 go build -buildmode=c-archive -o libdiscord.a
