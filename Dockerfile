FROM alpine:edge as dev

RUN echo "http://dl-cdn.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories;

RUN apk --no-cache add \
    gcc \
    g++ \
    make\
    binutils \
    enet \
    enet-dev \
    sqlite-dev \
    mariadb-connector-c \
    mariadb-connector-c-dev

COPY src/ /src/

RUN cd src; make -f Makefile.alpine

FROM alpine:edge

RUN ls -la

COPY --from=dev /remod64 /remod64

RUN echo "http://dl-cdn.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories;

RUN apk --no-cache add \
    enet \
    libstdc++ \
    sqlite-libs \
    mariadb-connector-c \
    libgcc
    
ENTRYPOINT ["/bin/sh"]