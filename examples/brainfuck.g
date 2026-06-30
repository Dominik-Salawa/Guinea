include io

function main(): int
    print("Brainfuck code:\n")
    const bf:    string = input()
    const cells: {char} = {30000}
    var   cursor:  int  = 0

    const stack: {int}    = {1024}
    var cursor_stack: int = 0

    for var i: int = 0; i < bf.length; i = i + 1 do
        if bf[i] == "+" then
            cells[cursor] = cells[cursor] + 1
            continue
        end
        if bf[i] == "-" then
            cells[cursor] = cells[cursor] - 1
            continue
        end
        if bf[i] == ">" then
            cursor = cursor + 1
            if cursor > 29999 then
                cursor = 0
            end
            continue
        end
        if bf[i] == "<" then
            cursor = cursor - 1
            if cursor < 0 then
                cursor = 29999
            end
            continue
        end
        if bf[i] == "," then
            cells[cursor] = input()[0]
            continue
        end
        if bf[i] == "." then
            print("%v", cells[cursor])
            continue
        end
        if bf[i] == "[" then
            if cursor_stack >= 1024 then
                println("Error: Stack error, too many loops!")
                return 1
            end
            stack[cursor_stack] = i
            cursor_stack = cursor_stack + 1
            
            if cells[cursor] == 0 then
                i = i + 1
                var nest: int = 0
                while not (bf[i] == "]" and nest == 0) and i < length(bf) do
                    if bf[i] == "[" then nest = nest + 1 end

                    if nest > 0 and bf[i] == "]": 
                        nest = nest - 1
                    
                    i = i+1
                end
            end

            continue
        end
        if bf[i] == "]" then
            if cursor_stack == 0 then
                println("Error: Stack error, no loop was encountered but end token recieved!")
                return 1
            end

            cursor_stack = cursor_stack - 1
            i = stack[cursor_stack]

            continue
        end
    end

    return 0
end