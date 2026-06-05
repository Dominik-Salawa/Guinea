const keywords = [
    "const", "var", "function", 
    "imul",

    "if", "while", "for",
    "repeat", "scope", "end",
    "break", "return", "continue",

    "class", "public", "private", "rom"
];
const datatype = [
    "dyn", "int",
    "string", "char",
    "float", "bool",
    "array", "list",
    "auto", "void",
    "nil", "true",
    "false",
    
    "i32", "i64",
    "f32", "f64",
    ""
];
const identifier = "abcdefghijklmnopqrstuvwxyz1234567890_";
const number     = "1234567890";
const codeblocks = document.getElementsByTagName("pre");

const stringcolor   = "#f2975e";
const datatypecolor = "#22d4bc";
const keywordcolor  = "#d60024";
const functioncolor = "#e3d949";

function isidentifier(token)
{
    if (number.includes(token[0])) return false;
    for (var ch of token) {
        if (!identifier.includes(ch)) return false;
    }
    return true;
}

function lex(text)
{
    var final = "";
    var token = "";
    var i = -1;

    function flush()
    {
        if (token === "") { return; }

        if (token === "<") {
            token = "&lt";
        } else if (token === ">") {
            token = "&gt";
        } else if (datatype.includes(token)) {
            token = `<span style="color:`+datatypecolor+`;">` + token + `</span>`;
        } else if (keywords.includes(token)) {
            token = `<span style="color:`+keywordcolor+`;">` + token + `</span>`;
        } else if (isidentifier(token)) {
            var isfunc = false;

            for (var j = i; j < text.length; j++) {
                var ch = text[j];
                
                if (identifier.includes(ch)) break;
                if (ch == ' ' || ch == '\n' || ch == '\t') continue;
                if (ch == '(') {
                    isfunc = true;
                    break;
                }
                break
            }

            if (isfunc) {
                token = `<span style="color:`+functioncolor+`;">` + token + `</span>`;
                console.log();
            }
        }

        final += token;
        token = "";
    }

    var doing_string = false;
    var doing_char   = false;
    for (let char of text) {
        i++;
        if (char === "\"" && doing_char == false) {
            console.log(doing_char, doing_string, i, text[i], text[i-1])
            flush();
            if (!doing_string) {
                doing_string = true;
                final += `<span style="color:`+stringcolor+`;">"`;
            } else if (text[i-1] != "\\") {
                doing_string = false;
                final += `"</span>`;
            } else {
                final += "\"";
            }
            continue;
        }
        if (char === "'" && !doing_string) {
            flush();
            if (!doing_char) {
                doing_char = true;
                final += `<span style="color:`+stringcolor+`;">'`;
            } else if (text[i-1] != "\\") {
                doing_char = false;
                final += `'</span>`;
            } else {
                final += "\'";
            }
            continue;
        }
        if (identifier.includes(char)) {
            token += char;
            continue;
        }
        flush();
        token += char;
        flush();
    }

    flush();

    return final;
}

for (var codeblock of codeblocks) {
    console.log(codeblock.textContent);
    const final = lex(codeblock.textContent);
    codeblock.innerHTML = (final);
}