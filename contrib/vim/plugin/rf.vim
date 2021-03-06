if exists('loaded_rf')
    finish
endif

let loaded_rf = 1
let g:rf_prefix = ""

set errorformat+=%f

function! g:RFGrepR(args)
    " let results = system('grep -nrI ' . shellescape(a:args) . " $(rf -- " . shellescape(a:ft) . ")")
    let results = system("rf -s'' | xargs grep -nrI '" . shellescape(a:args) . "'")
    cgete results | copen 9
endfunction

function! g:RFPrompt()
    echo "RF: "

    let l:rf_pattern = ''
    let l:rf_cmd = 0
    let l:rf_results = []

    while 1
        let c = getchar()

        if c == 13
            if l:rf_pattern == ''
                cclose
                break
            endif

            let i = getqflist({'idx' : 0}).idx

            if len(l:rf_results) > 0
                cclose
                execute('e ' . l:rf_results[i - 1])
            endif

            break
        elseif c == "\<BS>"
            let l:rf_pattern = l:rf_pattern[0:-2]
        elseif c == "\<UP>"
            let i = getqflist({'idx' : 0}).idx

            if i > 0
                call setqflist([], 'a', {'idx' : i - 1})
                redraw
            endif

            let l:rf_cmd = 1
        elseif c == "\<DOWN>"
            let i = getqflist({'idx' : 0}).idx
            call setqflist([], 'a', {'idx' : i + 1})
            redraw
            let l:rf_cmd = 1
        else
            let ch = nr2char(c)
            let l:rf_pattern = l:rf_pattern . ch
        endif

        if l:rf_cmd == 0
            echo "RF: " . l:rf_pattern
            redraw

            if len(g:rf_prefix)
                let results = system('rf -l9 -d ' . shellescape(g:rf_prefix) . ' -s -- ' . shellescape(l:rf_pattern))
            else
                let results = system('rf -l9 -ws -- ' . shellescape(l:rf_pattern))
            endif

            let results_list = split(results, '\n')
            let l:rf_results = results_list

            if len(results_list) > 0
                if len(results_list) < 9
                    execute "cgete results | copen " . len(results_list)
                else
                    cgete results | copen 9
                endif

                redraw
            else
                cexpr []
                cclose
            endif
        else
            let l:rf_cmd = 0
        endif
    endwhile
endfunction

command! -nargs=* RF call RFPrompt()
command! -nargs=* RFGrep call RFGrepR(<f-args>)
