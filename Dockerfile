FROM alpine:3.10.0 as dev
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
RUN cd src; make -f Makefile.alpine

FROM alpine:3.10.0
EXPOSE  28785 28786 28784 27070
WORKDIR /remod
COPY --from=dev remod64 remod64
COPY scripts scripts/
COPY maps maps/
COPY auth.cfg GeoIP.dat permbans.cfg ./
COPY server-init.cfg.default server-init.cfg
RUN apk --no-cache add \
    libstdc++ \
    sqlite-libs \
    mariadb-connector-c \
    libgcc
CMD ["./remod64"]