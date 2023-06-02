FROM ubuntu:20.04
MAINTAINER arestabrucs

ENV DEBIAN_FRONTEND=nonintercative
RUN apt update -qq && apt install -y --no-install-recommends \
    curl \
    git \
    wget \
    zip unzip \
    gpg-agent \
    python3-mapnik \
    mapnik-utils \
    python3-mapnik \
    python3-psycopg2 \
    python3-yaml \
    python-is-python3 \
    python3-queuelib \
    python3-pip \
    npm \
    osmium-tool \
    software-properties-common \
    build-essential \
    gdal-bin \
  && pip install pillow shapely matplotlib \
  && rm -rf /var/lib/apt/lists/*