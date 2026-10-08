"""Source-locked enum representation proofs, not whole-owner ABI acceptance."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[2]
GAME = REPO / "GeneralsMD/Code/GameEngine"


def clean(text):
    return re.sub(r'/\*.*?\*/|//[^\n]*',
                  lambda m: " " + "\n" * m[0].count("\n"), text, flags=re.DOTALL)


def enum_source(text, name):
    pattern = r'\benum\s+' + re.escape(name) + r'\s*(?::\s*(\w+)\s*)?\{([^}]+)\}'
    matches = list(re.finditer(pattern, clean(text), re.DOTALL))
    if len(matches) != 1:
        raise ValueError(f"{name}: expected one source definition")
    return matches[0][1], matches[0][2]


def enum_declarations(text):
    pattern = re.compile(r'\benum\s+(?:(?:class|struct)\s+)?(\w+)\s*(?::\s*(\w+)\s*)?([;{])')
    for match in pattern.finditer(text):
        # C++20 using-enum imports are not enum forward declarations. Do not
        # waive actual unfixed forwards or scoped definitions while recognizing it.
        if re.search(r'\busing\s*$', text[:match.start()]):
            continue
        yield match.groups()


def checked_domains():
    manifest = json.loads((REPO / "tests/original/enum_domains.json").read_text())
    domains = manifest["domains"]
    if len(domains) != 49 or len({r["name"] for r in domains}) != 49:
        raise ValueError("incomplete/duplicate domain census")
    expected = {r["name"]: r["underlying"] for r in domains}
    result = []
    for row in domains:
        path = REPO / row["header"]
        if not path.is_relative_to(GAME / "Include"):
            raise ValueError("definition outside game-owned headers")
        underlying, body = enum_source(path.read_text(), row["name"])
        digest = hashlib.sha256(" ".join(body.split()).encode()).hexdigest()
        if digest != row["enumerator_body_sha256"] or underlying != row["underlying"]:
            raise ValueError(f"{row['name']}: source values/representation changed")
        result.append((row["name"], underlying, body))
    counts = {name: 0 for name in expected}
    synthetic = 'using enum Imported; using\n enum AlsoImported; enum class Fixed : Int { X }; enum Unfixed;'
    if list(enum_declarations(synthetic)) != [('Fixed', 'Int', '{'), ('Unfixed', None, ';')]:
        raise ValueError("declaration inventory must retain fixed/scoped and unfixed-forward checks")
    for root in (GAME / "Include", GAME / "Source"):
        for path in root.rglob("*"):
            if path.suffix not in (".h", ".hpp", ".cpp", ".c"):
                continue
            text = clean(path.read_text(errors="replace"))
            if re.search(r'\b(?:HackerAttackMode|ObjectStatusType)\b', text):
                raise ValueError(f"{path.relative_to(REPO)}: unresolved legacy type")
            for name, underlying, kind in enum_declarations(text):
                if kind == ";" and underlying is None:
                    raise ValueError(f"{path.relative_to(REPO)}: unfixed {name} declaration")
                if name in expected:
                    if underlying != expected[name]:
                        raise ValueError(f"{path.relative_to(REPO)}: inconsistent {name}")
                    if kind == "{":
                        counts[name] += 1
    if counts != {name: 1 for name in expected}:
        raise ValueError("definition ownership mismatch")
    return result


def proof_source(domains):
    prefix = '''#include "Lib/BaseType.h"
#include <array>
#include <bit>
#include <cstddef>
#include <exception>
#include <type_traits>
'''
    old = "namespace prior {\n" + "\n".join(
        f"enum {name} {{{body}}};" for name, _, body in domains) + "\n}\n"
    new = "namespace native {\n" + "\n".join(
        f"enum {name} : {underlying} {{{body}}};" for name, underlying, body in domains) + "\n}\n"
    proof = '''template<class E> struct Layout {
    unsigned char prefix; Real weight; E value; unsigned char tail; void* owner;
};
template<class E, class Old, class Raw> void prove() {
    static_assert(sizeof(E) == 4 && sizeof(E) == sizeof(Old));
    static_assert(alignof(E) == alignof(Old));
    static_assert(std::is_same_v<std::underlying_type_t<E>,Raw>);
    static_assert(std::is_same_v<std::underlying_type_t<Old>,Raw>);
    static_assert(sizeof(Layout<E>) == sizeof(Layout<Old>));
    static_assert(alignof(Layout<E>) == alignof(Layout<Old>));
    static_assert(offsetof(Layout<E>, value) == offsetof(Layout<Old>, value));
    static_assert(offsetof(Layout<E>, tail) == offsetof(Layout<Old>, tail));
    static_assert(offsetof(Layout<E>, owner) == offsetof(Layout<Old>, owner));
    constexpr std::array<UnsignedInt,8> bits{0,1,0x7fffff,0x800000,0x7fffffff,
        0x80000000,0xfffffffe,0xffffffff};
    for (const auto raw : bits) {
        const E value = std::bit_cast<E>(raw);
        const Layout<E> stored{0xa5, 0.5f, value, 0x5a, nullptr};
        if (std::bit_cast<UnsignedInt>(stored.value) != raw ||
            static_cast<UnsignedInt>(static_cast<Raw>(stored.value)) != raw)
            std::terminate();
    }
}
int main() {
'''
    calls = "\n".join(f"prove<native::{name},prior::{name},{underlying}>();"
                      for name, underlying, _ in domains)
    return prefix + old + new + proof + calls + "\n}\n"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--sanitize", action="store_true")
    parser.add_argument("--alternate", action="store_true")
    args = parser.parse_args()
    domains = checked_domains()
    source = proof_source(domains)
    with tempfile.TemporaryDirectory(prefix="zh-enum-proof-") as directory:
        binary = str(Path(directory) / "proof")
        command = [args.compiler, "-std=c++20", "-O1", "-g",
                   "-I" + str(REPO / "GeneralsMD/Code/Libraries/Include")]
        if args.sanitize:
            command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        if args.alternate:
            command += ["-DALLOW_SURRENDER", "-DALLOW_DEMORALIZE"]
        subprocess.run(command + ["-x", "c++", "-", "-o", binary],
                       input=source, text=True, check=True)
        environment = os.environ.copy()
        environment["ASAN_OPTIONS"] = "detect_leaks=1:halt_on_error=1"
        environment["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
        subprocess.run([binary], env=environment, check=True)
    print("source-locked enum domains: 49 PASS; whole-owner ABI pending")


if __name__ == "__main__":
    main()
