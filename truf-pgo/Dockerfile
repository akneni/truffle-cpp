# Pull the parent image
FROM ubuntu:latest
COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /bin/
# FROM ghcr.io/astral-sh/uv:latest

# Set the working directory
WORKDIR /code

COPY . .

RUN apt update -y
RUN apt install build-essential -y
RUN apt install gcc -y

# RUN apt install python3 -y
# RUN apt install python3-pymongo -y

# RUN apt install curl -y
# RUN curl -LsSf https://astral.sh/uv/install.sh | sh
# RUN uv python install

CMD ["uv", "run", "--with", "pymongo", "/docker-volume/benchmark.py", "--trials=3"]
