FROM debian:11.1-slim as builder

WORKDIR /build

RUN apt-get update \
    && apt-get install -y \
        gcc \
        g++ \
        make \
        libsqlite3-dev \
        libmariadb-dev-compat \
        autoconf \
        libtool \
        golang \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY src src/

RUN make -C src SQLITE3_USE_SYSTEM_LIBS=true MYSQL_USE_SYSTEM_LIBS=true USE_EXPEREMENTAL_DISCORD=true

FROM debian:11.1-slim

WORKDIR /remod

# Expose laninfo, server, serverinfo, rcon
EXPOSE 28784 28785 28786 27070

COPY --from=builder /build/remod /remod/remod
COPY auth.cfg GeoIP.dat GeoLite2-Country.mmdb permbans.cfg /remod/
COPY server-init.cfg.default /remod/server-init.cfg
COPY scripts /remod/scripts/
COPY maps /remod/maps/

RUN apt-get update \
    && apt-get -y install \
    libsqlite3-0 \
    libmariadb3

CMD ["/remod/remod"]
