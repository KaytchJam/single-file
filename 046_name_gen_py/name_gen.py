import random

ROOT: str = '*'
TERMINATOR: str = '\0'
MAX_CHUNK_SIZE: int = 5

def build_markov(names: list[str]):
    markov: dict[str, list[str]] = { ROOT : [] }

    for name in names:
        prev: str = ROOT
        built: str = prev

        for idx, letter in enumerate(name):
            built += letter

            for size in range(0, min(len(built), MAX_CHUNK_SIZE)):
                chunk: str = built[-(size + 1):-1]
                if chunk not in markov:
                    markov[chunk] = []
                markov[chunk].append(letter)

                if idx == len(name) - 1:
                    markov[chunk].append(TERMINATOR)

            prev = letter

        if prev not in markov:
            markov[prev] = []
        markov[prev].append(TERMINATOR)

    return markov

def get_next(markov: dict[str, list[str]], character: str) -> str:
    if not character in markov:
        return TERMINATOR
    bag: list[str] = markov[character]
    return random.choice(bag)

def generate_name(markov: dict[str, list[str]]) -> str:
    name: str = ROOT
    while name[-1] != TERMINATOR:
        chunk_size: int = random.randint(1, min(len(name), MAX_CHUNK_SIZE))
        char: str = get_next(markov, name[-1 * chunk_size:])
        name += char
    return name[1:-1]

def main():
    NUM_NAMES_TO_GENERATE: int = 10
    names: list[str] = []
    with open("./names.txt") as file:
        for line in file:
            stripped_line: str = line.strip()
            if len(stripped_line) > 0:
                names.append(stripped_line.lower())

    markov: dict[str, list[str]] = build_markov(names)

    for idx in range(NUM_NAMES_TO_GENERATE):
        name: str = generate_name(markov)
        print(f"{idx + 1}) {name.capitalize()}")

if __name__ == "__main__":
    main()
