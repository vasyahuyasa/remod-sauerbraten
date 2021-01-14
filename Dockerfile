FROM alpine:3.12.1 as builder

RUN apk --no-cache add \
    gcc \
    g++ \
    make\
    binutils \
    sqlite-dev \
    mariadb-connector-c \
    mariadb-connector-c-dev \
    autoconf \
    automake \
    libtool
COPY src src/

RUN cd src; make SQLITE3_USE_SYSTEM_LIBS=true MYSQL_USE_SYSTEM_LIBS=true

FROM alpine:3.12.1

WORKDIR /remod

# Expose laninfo, server, serverinfo, rcon
EXPOSE 28784 28785 28786 27070

COPY --from=builder remod64 remod64
COPY scripts scripts/
COPY maps maps/
COPY auth.cfg GeoIP.dat GeoLite2-Country.mmdb permbans.cfg ./
COPY server-init.cfg.default server-init.cfg

RUN apk --no-cache add \
    libstdc++ \
    sqlite-libs \
    mariadb-connector-c \
    libgcc

ENTRYPOINT ["/remod/remod64"]
