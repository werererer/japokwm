let s:cpo_save=&cpo
set cpo&vim
inoremap <silent> <expr> <Plug>(coc-snippets-expand-jump) coc#_insert_key('notify', 'snippets-expand-jump', 1)
inoremap <silent> <expr> <Plug>(coc-snippets-expand) coc#_insert_key('notify', 'snippets-expand', 1)
inoremap <silent> <SNR>46_AutoPairsReturn =AutoPairsReturn()
inoremap <silent> <Plug>CocRefresh =coc#_complete()
inoremap <M-l> :Limelight!!
inoremap <M-Space> :Goyo
inoremap <F6> :call vimspector#Launch()
inoremap <F5> :make!a
inoremap <silent> <expr> <C-Space> coc#refresh()
vmap <NL> <Plug>(coc-snippets-select)
map  <Plug>(ctrlp)
nnoremap < 5<
nnoremap > 5>
tnoremap  
nnoremap  o :NERDTreeToggleVCS
nnoremap  ce :ProjectRootExe tabe CMakeLists.txt
vnoremap <silent>  cs "ky:ThesaurusQueryReplace k
nnoremap <silent>  cs :ThesaurusQueryReplaceCurrentWord
nmap  un <Plug>(UnicodeSwapCompleteName)
nnoremap <silent>  h :noh
nnoremap  ss :exec "source ". g:sessionFile
nnoremap  ms :exec "mksession! ". g:sessionFile
nnoremap  b :call toggleBg#toggleColor()
nnoremap  na :normal diavIapvalaoP
nnoremap  la :normal dIavIlapvinap
nnoremap  eh :exec "tabe ~/.config/nvim/after/syntax/". &filetype. ".vim"
nnoremap  ef :exec "tabe ~/.config/nvim/ftplugin/". &filetype. ".vim"
nnoremap  sv :source $MYVIMRC
nnoremap  ev :tabe $MYVIMRC
nnoremap  ea :tabe ~/.config/nvim/autoload
nnoremap  dc :call vimspector#Continue()
nnoremap  tb :call vimspector#ToggleBreakpoint()
nnoremap  dr :call vimspector#Reset()
nnoremap  ed :tabe .vimspector.json
nnoremap  tg :SignifyToggle
nmap  f :CocFix
nmap <silent>  a <Plug>(coc-references-used)
nmap  e <Plug>(coc-refactor)
nmap  r <Plug>(coc-rename)
noremap  fs :tabe % :CocList snippets
nnoremap  es :tabe % :CocCommand snippets.editSnippets
nnoremap  l :call LocListToggle()
nnoremap  q :call QuickfixToggle()
nnoremap $ g$
omap <silent> % <Plug>(MatchitOperationForward)
xmap <silent> % <Plug>(MatchitVisualForward)
nmap <silent> % <Plug>(MatchitNormalForward)
nnoremap 0 g0
xnoremap <silent> @(targets) :call targets#do()
onoremap <silent> @(targets) :call targets#do()
xmap <expr> A targets#e('o', 'A', 'A')
omap <expr> A targets#e('o', 'A', 'A')
xmap <expr> I targets#e('o', 'I', 'I')
omap <expr> I targets#e('o', 'I', 'I')
xmap S <Plug>VSurround
omap <silent> [% <Plug>(MatchitOperationMultiBackward)
xmap <silent> [% <Plug>(MatchitVisualMultiBackward)
nmap <silent> [% <Plug>(MatchitNormalMultiBackward)
nmap [C 9999[c
nmap [c <Plug>(signify-prev-hunk)
nnoremap [X ?>>>>>>> HEAD>CR>
nnoremap [= ?======= HEAD=CR>
nnoremap [x ?<<<<<<< HEAD
nmap <silent> [w <Plug>(coc-diagnostic-next)
vnoremap <silent> \cs "ky:ThesaurusQueryReplace k
nnoremap <silent> \cs :ThesaurusQueryReplaceCurrentWord
omap <silent> ]% <Plug>(MatchitOperationMultiForward)
xmap <silent> ]% <Plug>(MatchitVisualMultiForward)
nmap <silent> ]% <Plug>(MatchitNormalMultiForward)
nmap ]C 9999]c
nmap ]c <Plug>(signify-next-hunk)
nnoremap ]X />>>>>>> HEAD>CR>
nnoremap ]= /======= HEAD=CR>
nnoremap ]x /<<<<<<< HEAD
nmap <silent> ]w <Plug>(coc-diagnostic-prev)
xmap a% <Plug>(MatchitVisualTextObject)
xmap <expr> a targets#e('o', 'a', 'a')
omap <expr> a targets#e('o', 'a', 'a')
omap ac <Plug>(coc-classobj-a)
xmap ac <Plug>(coc-classobj-a)
omap af <Plug>(coc-funcobj-a)
xmap af <Plug>(coc-funcobj-a)
nmap cS <Plug>CSurround
nmap cs <Plug>Csurround
nmap ds <Plug>Dsurround
nnoremap dl :normal ^d$"_gJ
vmap gx <Plug>NetrwBrowseXVis
nmap gx <Plug>NetrwBrowseX
omap <silent> g% <Plug>(MatchitOperationBackward)
xmap <silent> g% <Plug>(MatchitVisualBackward)
nmap <silent> g% <Plug>(MatchitNormalBackward)
xmap gS <Plug>VgSurround
nmap gcu <Plug>Commentary<Plug>Commentary
nmap gcc <Plug>CommentaryLine
omap gc <Plug>Commentary
nmap gc <Plug>Commentary
xmap gc <Plug>Commentary
nmap <silent> gy <Plug>(coc-type-definition)
nmap <silent> gD <Plug>(coc-declaration)
nmap <silent> gd <Plug>(coc-definition)
xmap <expr> i targets#e('o', 'i', 'i')
omap <expr> i targets#e('o', 'i', 'i')
omap ic <Plug>(coc-classobj-i)
xmap ic <Plug>(coc-classobj-i)
omap if <Plug>(coc-funcobj-i)
xmap if <Plug>(coc-funcobj-i)
nnoremap j gj
nnoremap k gk
nnoremap td :tabclose
nnoremap t% :tabedit %
nmap ySS <Plug>YSsurround
nmap ySs <Plug>YSsurround
nmap yss <Plug>Yssurround
nmap yS <Plug>YSurround
nmap ys <Plug>Ysurround
nnoremap <SNR>133_: :=v:count ? v:count : ''
nnoremap <F1> :Execute
vnoremap <silent> <Plug>(coc-explorer-key-v->>) :call coc#rpc#request('doKeymap', ['explorer-key-v->>'])
nnoremap <silent> <Plug>(coc-explorer-key-n->>) :call coc#rpc#request('doKeymap', ['explorer-key-n->>'])
vnoremap <silent> <Plug>(coc-explorer-key-v-<<) :call coc#rpc#request('doKeymap', ['explorer-key-v-<<'])
nnoremap <silent> <Plug>(coc-explorer-key-n-<<) :call coc#rpc#request('doKeymap', ['explorer-key-n-<<'])
vnoremap <silent> <Plug>(coc-explorer-key-v-]c) :call coc#rpc#request('doKeymap', ['explorer-key-v-]c'])
nnoremap <silent> <Plug>(coc-explorer-key-n-]c) :call coc#rpc#request('doKeymap', ['explorer-key-n-]c'])
vnoremap <silent> <Plug>(coc-explorer-key-v-[c) :call coc#rpc#request('doKeymap', ['explorer-key-v-[c'])
nnoremap <silent> <Plug>(coc-explorer-key-n-[c) :call coc#rpc#request('doKeymap', ['explorer-key-n-[c'])
vnoremap <silent> <Plug>(coc-explorer-key-v-]D) :call coc#rpc#request('doKeymap', ['explorer-key-v-]D'])
nnoremap <silent> <Plug>(coc-explorer-key-n-]D) :call coc#rpc#request('doKeymap', ['explorer-key-n-]D'])
vnoremap <silent> <Plug>(coc-explorer-key-v-[D) :call coc#rpc#request('doKeymap', ['explorer-key-v-[D'])
nnoremap <silent> <Plug>(coc-explorer-key-n-[D) :call coc#rpc#request('doKeymap', ['explorer-key-n-[D'])
vnoremap <silent> <Plug>(coc-explorer-key-v-]d) :call coc#rpc#request('doKeymap', ['explorer-key-v-]d'])
nnoremap <silent> <Plug>(coc-explorer-key-n-]d) :call coc#rpc#request('doKeymap', ['explorer-key-n-]d'])
vnoremap <silent> <Plug>(coc-explorer-key-v-[d) :call coc#rpc#request('doKeymap', ['explorer-key-v-[d'])
nnoremap <silent> <Plug>(coc-explorer-key-n-[d) :call coc#rpc#request('doKeymap', ['explorer-key-n-[d'])
vnoremap <silent> <Plug>(coc-explorer-key-v-]m) :call coc#rpc#request('doKeymap', ['explorer-key-v-]m'])
nnoremap <silent> <Plug>(coc-explorer-key-n-]m) :call coc#rpc#request('doKeymap', ['explorer-key-n-]m'])
vnoremap <silent> <Plug>(coc-explorer-key-v-[m) :call coc#rpc#request('doKeymap', ['explorer-key-v-[m'])
nnoremap <silent> <Plug>(coc-explorer-key-n-[m) :call coc#rpc#request('doKeymap', ['explorer-key-n-[m'])
vnoremap <silent> <Plug>(coc-explorer-key-v-]i) :call coc#rpc#request('doKeymap', ['explorer-key-v-]i'])
nnoremap <silent> <Plug>(coc-explorer-key-n-]i) :call coc#rpc#request('doKeymap', ['explorer-key-n-]i'])
vnoremap <silent> <Plug>(coc-explorer-key-v-[i) :call coc#rpc#request('doKeymap', ['explorer-key-v-[i'])
nnoremap <silent> <Plug>(coc-explorer-key-n-[i) :call coc#rpc#request('doKeymap', ['explorer-key-n-[i'])
vnoremap <silent> <Plug>(coc-explorer-key-v-]]) :call coc#rpc#request('doKeymap', ['explorer-key-v-]]'])
nnoremap <silent> <Plug>(coc-explorer-key-n-]]) :call coc#rpc#request('doKeymap', ['explorer-key-n-]]'])
vnoremap <silent> <Plug>(coc-explorer-key-v-[[) :call coc#rpc#request('doKeymap', ['explorer-key-v-[['])
nnoremap <silent> <Plug>(coc-explorer-key-n-[[) :call coc#rpc#request('doKeymap', ['explorer-key-n-[['])
vnoremap <silent> <Plug>(coc-explorer-key-v-gb) :call coc#rpc#request('doKeymap', ['explorer-key-v-gb'])
nnoremap <silent> <Plug>(coc-explorer-key-n-gb) :call coc#rpc#request('doKeymap', ['explorer-key-n-gb'])
vnoremap <silent> <Plug>(coc-explorer-key-v-gf) :call coc#rpc#request('doKeymap', ['explorer-key-v-gf'])
nnoremap <silent> <Plug>(coc-explorer-key-n-gf) :call coc#rpc#request('doKeymap', ['explorer-key-n-gf'])
vnoremap <silent> <Plug>(coc-explorer-key-v-F) :call coc#rpc#request('doKeymap', ['explorer-key-v-F'])
nnoremap <silent> <Plug>(coc-explorer-key-n-F) :call coc#rpc#request('doKeymap', ['explorer-key-n-F'])
vnoremap <silent> <Plug>(coc-explorer-key-v-f) :call coc#rpc#request('doKeymap', ['explorer-key-v-f'])
nnoremap <silent> <Plug>(coc-explorer-key-n-f) :call coc#rpc#request('doKeymap', ['explorer-key-n-f'])
vnoremap <silent> <Plug>(coc-explorer-key-v-gd) :call coc#rpc#request('doKeymap', ['explorer-key-v-gd'])
nnoremap <silent> <Plug>(coc-explorer-key-n-gd) :call coc#rpc#request('doKeymap', ['explorer-key-n-gd'])
vnoremap <silent> <Plug>(coc-explorer-key-v-X) :call coc#rpc#request('doKeymap', ['explorer-key-v-X'])
nnoremap <silent> <Plug>(coc-explorer-key-n-X) :call coc#rpc#request('doKeymap', ['explorer-key-n-X'])
vnoremap <silent> <Plug>(coc-explorer-key-v-[esc]) :call coc#rpc#request('doKeymap', ['explorer-key-v-[esc]'])
nnoremap <silent> <Plug>(coc-explorer-key-n-[esc]) :call coc#rpc#request('doKeymap', ['explorer-key-n-[esc]'])
vnoremap <silent> <Plug>(coc-explorer-key-v-q) :call coc#rpc#request('doKeymap', ['explorer-key-v-q'])
nnoremap <silent> <Plug>(coc-explorer-key-n-q) :call coc#rpc#request('doKeymap', ['explorer-key-n-q'])
vnoremap <silent> <Plug>(coc-explorer-key-v-?) :call coc#rpc#request('doKeymap', ['explorer-key-v-?'])
nnoremap <silent> <Plug>(coc-explorer-key-n-?) :call coc#rpc#request('doKeymap', ['explorer-key-n-?'])
vnoremap <silent> <Plug>(coc-explorer-key-v-R) :call coc#rpc#request('doKeymap', ['explorer-key-v-R'])
nnoremap <silent> <Plug>(coc-explorer-key-n-R) :call coc#rpc#request('doKeymap', ['explorer-key-n-R'])
vnoremap <silent> <Plug>(coc-explorer-key-v-.) :call coc#rpc#request('doKeymap', ['explorer-key-v-.'])
nnoremap <silent> <Plug>(coc-explorer-key-n-.) :call coc#rpc#request('doKeymap', ['explorer-key-n-.'])
vnoremap <silent> <Plug>(coc-explorer-key-v-r) :call coc#rpc#request('doKeymap', ['explorer-key-v-r'])
nnoremap <silent> <Plug>(coc-explorer-key-n-r) :call coc#rpc#request('doKeymap', ['explorer-key-n-r'])
vnoremap <silent> <Plug>(coc-explorer-key-v-A) :call coc#rpc#request('doKeymap', ['explorer-key-v-A'])
nnoremap <silent> <Plug>(coc-explorer-key-n-A) :call coc#rpc#request('doKeymap', ['explorer-key-n-A'])
vnoremap <silent> <Plug>(coc-explorer-key-v-a) :call coc#rpc#request('doKeymap', ['explorer-key-v-a'])
nnoremap <silent> <Plug>(coc-explorer-key-n-a) :call coc#rpc#request('doKeymap', ['explorer-key-n-a'])
vnoremap <silent> <Plug>(coc-explorer-key-v-D) :call coc#rpc#request('doKeymap', ['explorer-key-v-D'])
nnoremap <silent> <Plug>(coc-explorer-key-n-D) :call coc#rpc#request('doKeymap', ['explorer-key-n-D'])
vnoremap <silent> <Plug>(coc-explorer-key-v-d) :call coc#rpc#request('doKeymap', ['explorer-key-v-d'])
nnoremap <silent> <Plug>(coc-explorer-key-n-d) :call coc#rpc#request('doKeymap', ['explorer-key-n-d'])
vnoremap <silent> <Plug>(coc-explorer-key-v-p) :call coc#rpc#request('doKeymap', ['explorer-key-v-p'])
nnoremap <silent> <Plug>(coc-explorer-key-n-p) :call coc#rpc#request('doKeymap', ['explorer-key-n-p'])
vnoremap <silent> <Plug>(coc-explorer-key-v-x) :call coc#rpc#request('doKeymap', ['explorer-key-v-x'])
nnoremap <silent> <Plug>(coc-explorer-key-n-x) :call coc#rpc#request('doKeymap', ['explorer-key-n-x'])
vnoremap <silent> <Plug>(coc-explorer-key-v-c) :call coc#rpc#request('doKeymap', ['explorer-key-v-c'])
nnoremap <silent> <Plug>(coc-explorer-key-n-c) :call coc#rpc#request('doKeymap', ['explorer-key-n-c'])
vnoremap <silent> <Plug>(coc-explorer-key-v-Y) :call coc#rpc#request('doKeymap', ['explorer-key-v-Y'])
nnoremap <silent> <Plug>(coc-explorer-key-n-Y) :call coc#rpc#request('doKeymap', ['explorer-key-n-Y'])
vnoremap <silent> <Plug>(coc-explorer-key-v-y) :call coc#rpc#request('doKeymap', ['explorer-key-v-y'])
nnoremap <silent> <Plug>(coc-explorer-key-n-y) :call coc#rpc#request('doKeymap', ['explorer-key-n-y'])
vnoremap <silent> <Plug>(coc-explorer-key-v-gp) :call coc#rpc#request('doKeymap', ['explorer-key-v-gp'])
nnoremap <silent> <Plug>(coc-explorer-key-n-gp) :call coc#rpc#request('doKeymap', ['explorer-key-n-gp'])
vnoremap <silent> <Plug>(coc-explorer-key-v-gs) :call coc#rpc#request('doKeymap', ['explorer-key-v-gs'])
nnoremap <silent> <Plug>(coc-explorer-key-n-gs) :call coc#rpc#request('doKeymap', ['explorer-key-n-gs'])
vnoremap <silent> <Plug>(coc-explorer-key-v-[bs]) :call coc#rpc#request('doKeymap', ['explorer-key-v-[bs]'])
nnoremap <silent> <Plug>(coc-explorer-key-n-[bs]) :call coc#rpc#request('doKeymap', ['explorer-key-n-[bs]'])
vnoremap <silent> <Plug>(coc-explorer-key-v-t) :call coc#rpc#request('doKeymap', ['explorer-key-v-t'])
nnoremap <silent> <Plug>(coc-explorer-key-n-t) :call coc#rpc#request('doKeymap', ['explorer-key-n-t'])
vnoremap <silent> <Plug>(coc-explorer-key-v-E) :call coc#rpc#request('doKeymap', ['explorer-key-v-E'])
nnoremap <silent> <Plug>(coc-explorer-key-n-E) :call coc#rpc#request('doKeymap', ['explorer-key-n-E'])
vnoremap <silent> <Plug>(coc-explorer-key-v-s) :call coc#rpc#request('doKeymap', ['explorer-key-v-s'])
nnoremap <silent> <Plug>(coc-explorer-key-n-s) :call coc#rpc#request('doKeymap', ['explorer-key-n-s'])
vnoremap <silent> <Plug>(coc-explorer-key-v-e) :call coc#rpc#request('doKeymap', ['explorer-key-v-e'])
nnoremap <silent> <Plug>(coc-explorer-key-n-e) :call coc#rpc#request('doKeymap', ['explorer-key-n-e'])
vnoremap <silent> <Plug>(coc-explorer-key-v-[cr]) :call coc#rpc#request('doKeymap', ['explorer-key-v-[cr]'])
nnoremap <silent> <Plug>(coc-explorer-key-n-[cr]) :call coc#rpc#request('doKeymap', ['explorer-key-n-[cr]'])
nnoremap <silent> <Plug>(coc-explorer-key-n-o) :call coc#rpc#request('doKeymap', ['explorer-key-n-o'])
vnoremap <silent> <Plug>(coc-explorer-key-v-[2-LeftMouse]) :call coc#rpc#request('doKeymap', ['explorer-key-v-[2-LeftMouse]'])
nnoremap <silent> <Plug>(coc-explorer-key-n-[2-LeftMouse]) :call coc#rpc#request('doKeymap', ['explorer-key-n-[2-LeftMouse]'])
vnoremap <silent> <Plug>(coc-explorer-key-v-gh) :call coc#rpc#request('doKeymap', ['explorer-key-v-gh'])
nnoremap <silent> <Plug>(coc-explorer-key-n-gh) :call coc#rpc#request('doKeymap', ['explorer-key-n-gh'])
vnoremap <silent> <Plug>(coc-explorer-key-v-gl) :call coc#rpc#request('doKeymap', ['explorer-key-v-gl'])
nnoremap <silent> <Plug>(coc-explorer-key-n-gl) :call coc#rpc#request('doKeymap', ['explorer-key-n-gl'])
vnoremap <silent> <Plug>(coc-explorer-key-v-K) :call coc#rpc#request('doKeymap', ['explorer-key-v-K'])
nnoremap <silent> <Plug>(coc-explorer-key-n-K) :call coc#rpc#request('doKeymap', ['explorer-key-n-K'])
vnoremap <silent> <Plug>(coc-explorer-key-v-J) :call coc#rpc#request('doKeymap', ['explorer-key-v-J'])
nnoremap <silent> <Plug>(coc-explorer-key-n-J) :call coc#rpc#request('doKeymap', ['explorer-key-n-J'])
vnoremap <silent> <Plug>(coc-explorer-key-v-l) :call coc#rpc#request('doKeymap', ['explorer-key-v-l'])
nnoremap <silent> <Plug>(coc-explorer-key-n-l) :call coc#rpc#request('doKeymap', ['explorer-key-n-l'])
vnoremap <silent> <Plug>(coc-explorer-key-v-h) :call coc#rpc#request('doKeymap', ['explorer-key-v-h'])
nnoremap <silent> <Plug>(coc-explorer-key-n-h) :call coc#rpc#request('doKeymap', ['explorer-key-n-h'])
vnoremap <silent> <Plug>(coc-explorer-key-v-gj) :call coc#rpc#request('doKeymap', ['explorer-key-v-gj'])
nnoremap <silent> <Plug>(coc-explorer-key-n-gj) :call coc#rpc#request('doKeymap', ['explorer-key-n-gj'])
vnoremap <silent> <Plug>(coc-explorer-key-v-gk) :call coc#rpc#request('doKeymap', ['explorer-key-v-gk'])
nnoremap <silent> <Plug>(coc-explorer-key-n-gk) :call coc#rpc#request('doKeymap', ['explorer-key-n-gk'])
vnoremap <silent> <Plug>(coc-explorer-key-v-[tab]) :call coc#rpc#request('doKeymap', ['explorer-key-v-[tab]'])
nnoremap <silent> <Plug>(coc-explorer-key-n-[tab]) :call coc#rpc#request('doKeymap', ['explorer-key-n-[tab]'])
vnoremap <silent> <Plug>(coc-explorer-key-v-*) :call coc#rpc#request('doKeymap', ['explorer-key-v-*'])
nnoremap <silent> <Plug>(coc-explorer-key-n-*) :call coc#rpc#request('doKeymap', ['explorer-key-n-*'])
vnoremap <silent> <Plug>(coc-snippets-select) :call coc#rpc#notify('doKeymap', ['snippets-select'])
xnoremap <silent> <Plug>(coc-convert-snippet) :call coc#rpc#notify('doKeymap', ['convert-snippet'])
noremap <silent> <M-Tab> :CocCommand clangd.switchSourceHeader
nnoremap <F7> :Make:Execute
nnoremap <F2> :ProjectRootExe Termdebug ../bin/out
vnoremap <silent> <Plug>NetrwBrowseXVis :call netrw#BrowseXVis()
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#BrowseX(expand((exists("g:netrw_gx")? g:netrw_gx : '<cfile>')),netrw#CheckIfRemote())
vmap <silent> <Plug>(MatchitVisualTextObject) <Plug>(MatchitVisualMultiBackward)o<Plug>(MatchitVisualMultiForward)
onoremap <silent> <Plug>(MatchitOperationMultiForward) :call matchit#MultiMatch("W",  "o")
onoremap <silent> <Plug>(MatchitOperationMultiBackward) :call matchit#MultiMatch("bW", "o")
vnoremap <silent> <Plug>(MatchitVisualMultiForward) :call matchit#MultiMatch("W",  "n")m'gv``
vnoremap <silent> <Plug>(MatchitVisualMultiBackward) :call matchit#MultiMatch("bW", "n")m'gv``
nnoremap <silent> <Plug>(MatchitNormalMultiForward) :call matchit#MultiMatch("W",  "n")
nnoremap <silent> <Plug>(MatchitNormalMultiBackward) :call matchit#MultiMatch("bW", "n")
onoremap <silent> <Plug>(MatchitOperationBackward) :call matchit#Match_wrapper('',0,'o')
onoremap <silent> <Plug>(MatchitOperationForward) :call matchit#Match_wrapper('',1,'o')
vnoremap <silent> <Plug>(MatchitVisualBackward) :call matchit#Match_wrapper('',0,'v')m'gv``
vnoremap <silent> <Plug>(MatchitVisualForward) :call matchit#Match_wrapper('',1,'v')m'gv``
nnoremap <silent> <Plug>(MatchitNormalBackward) :call matchit#Match_wrapper('',0,'n')
nnoremap <silent> <Plug>(MatchitNormalForward) :call matchit#Match_wrapper('',1,'n')
nnoremap <silent> <Plug>(ctrlp) :CtrlP
xnoremap <silent> <Plug>(QuickScopeToggle) :call quick_scope#Toggle()
nnoremap <silent> <Plug>(QuickScopeToggle) :call quick_scope#Toggle()
onoremap <silent> <Plug>(coc-classobj-a) :call coc#rpc#request('selectSymbolRange', [v:false, '', ['Interface', 'Struct', 'Class']])
onoremap <silent> <Plug>(coc-classobj-i) :call coc#rpc#request('selectSymbolRange', [v:true, '', ['Interface', 'Struct', 'Class']])
vnoremap <silent> <Plug>(coc-classobj-a) :call coc#rpc#request('selectSymbolRange', [v:false, visualmode(), ['Interface', 'Struct', 'Class']])
vnoremap <silent> <Plug>(coc-classobj-i) :call coc#rpc#request('selectSymbolRange', [v:true, visualmode(), ['Interface', 'Struct', 'Class']])
onoremap <silent> <Plug>(coc-funcobj-a) :call coc#rpc#request('selectSymbolRange', [v:false, '', ['Method', 'Function']])
onoremap <silent> <Plug>(coc-funcobj-i) :call coc#rpc#request('selectSymbolRange', [v:true, '', ['Method', 'Function']])
vnoremap <silent> <Plug>(coc-funcobj-a) :call coc#rpc#request('selectSymbolRange', [v:false, visualmode(), ['Method', 'Function']])
vnoremap <silent> <Plug>(coc-funcobj-i) :call coc#rpc#request('selectSymbolRange', [v:true, visualmode(), ['Method', 'Function']])
nnoremap <silent> <Plug>(coc-cursors-position) :call coc#rpc#request('cursorsSelect', [bufnr('%'), 'position', 'n'])
nnoremap <silent> <Plug>(coc-cursors-word) :call coc#rpc#request('cursorsSelect', [bufnr('%'), 'word', 'n'])
vnoremap <silent> <Plug>(coc-cursors-range) :call coc#rpc#request('cursorsSelect', [bufnr('%'), 'range', visualmode()])
nnoremap <silent> <Plug>(coc-refactor) :call       CocActionAsync('refactor')
nnoremap <silent> <Plug>(coc-command-repeat) :call       CocAction('repeatCommand')
nnoremap <silent> <Plug>(coc-float-jump) :call       coc#float#jump()
nnoremap <silent> <Plug>(coc-float-hide) :call       coc#float#close_all()
nnoremap <silent> <Plug>(coc-fix-current) :call       CocActionAsync('doQuickfix')
nnoremap <silent> <Plug>(coc-openlink) :call       CocActionAsync('openLink')
nnoremap <silent> <Plug>(coc-references-used) :call       CocActionAsync('jumpUsed')
nnoremap <silent> <Plug>(coc-references) :call       CocActionAsync('jumpReferences')
nnoremap <silent> <Plug>(coc-type-definition) :call       CocActionAsync('jumpTypeDefinition')
nnoremap <silent> <Plug>(coc-implementation) :call       CocActionAsync('jumpImplementation')
nnoremap <silent> <Plug>(coc-declaration) :call       CocActionAsync('jumpDeclaration')
nnoremap <silent> <Plug>(coc-definition) :call       CocActionAsync('jumpDefinition')
nnoremap <silent> <Plug>(coc-diagnostic-prev-error) :call       CocActionAsync('diagnosticPrevious', 'error')
nnoremap <silent> <Plug>(coc-diagnostic-next-error) :call       CocActionAsync('diagnosticNext',     'error')
nnoremap <silent> <Plug>(coc-diagnostic-prev) :call       CocActionAsync('diagnosticPrevious')
nnoremap <silent> <Plug>(coc-diagnostic-next) :call       CocActionAsync('diagnosticNext')
nnoremap <silent> <Plug>(coc-diagnostic-info) :call       CocActionAsync('diagnosticInfo')
nnoremap <silent> <Plug>(coc-format) :call       CocActionAsync('format')
nnoremap <silent> <Plug>(coc-rename) :call       CocActionAsync('rename')
nnoremap <Plug>(coc-codeaction-line) :call       CocActionAsync('codeAction',         'n')
nnoremap <Plug>(coc-codeaction) :call       CocActionAsync('codeAction',         '')
vnoremap <silent> <Plug>(coc-codeaction-selected) :call       CocActionAsync('codeAction',         visualmode())
vnoremap <silent> <Plug>(coc-format-selected) :call       CocActionAsync('formatSelected',     visualmode())
nnoremap <Plug>(coc-codelens-action) :call       CocActionAsync('codeLensAction')
nnoremap <Plug>(coc-range-select) :call       CocActionAsync('rangeSelect',     '', v:true)
vnoremap <silent> <Plug>(coc-range-select-backward) :call       CocActionAsync('rangeSelect',     visualmode(), v:false)
vnoremap <silent> <Plug>(coc-range-select) :call       CocActionAsync('rangeSelect',     visualmode(), v:true)
nnoremap <silent> <Plug>SurroundRepeat .
vmap <F4> <Plug>(MakeDigraph)
nmap <F4> <Plug>(MakeDigraph)
xnoremap <silent> <Plug>(Limelight) :Limelight
nnoremap <silent> <Plug>(Limelight) :set opfunc=limelight#operatorg@
nmap <silent> <Plug>CommentaryUndo :echoerr "Change your <Plug>CommentaryUndo map to <Plug>Commentary<Plug>Commentary"
nnoremap <silent> <Plug>VimspectorRunToCursor :call vimspector#RunToCursor()
nnoremap <silent> <Plug>VimspectorStepOut :call vimspector#StepOut()
nnoremap <silent> <Plug>VimspectorStepInto :call vimspector#StepInto()
nnoremap <silent> <Plug>VimspectorStepOver :call vimspector#StepOver()
nnoremap <silent> <Plug>VimspectorAddFunctionBreakpoint :call vimspector#AddFunctionBreakpoint( expand( '<cexpr>' ) )
nnoremap <silent> <Plug>VimspectorToggleConditionalBreakpoint :call vimspector#ToggleBreakpoint( { 'condition': input( 'Enter condition expression: ' ),   'hitCondition': input( 'Enter hit count expression: ' ) } )
nnoremap <silent> <Plug>VimspectorToggleBreakpoint :call vimspector#ToggleBreakpoint()
nnoremap <silent> <Plug>VimspectorPause :call vimspector#Pause()
nnoremap <silent> <Plug>VimspectorRestart :call vimspector#Restart()
nnoremap <silent> <Plug>VimspectorStop :call vimspector#Stop()
nnoremap <silent> <Plug>VimspectorContinue :call vimspector#Continue()
xnoremap <silent> <Plug>(signify-motion-outer-visual) :call sy#util#hunk_text_object(1)
onoremap <silent> <Plug>(signify-motion-outer-pending) :call sy#util#hunk_text_object(1)
xnoremap <silent> <Plug>(signify-motion-inner-visual) :call sy#util#hunk_text_object(0)
onoremap <silent> <Plug>(signify-motion-inner-pending) :call sy#util#hunk_text_object(0)
nnoremap <silent> <expr> <Plug>(signify-prev-hunk) &diff ? '[c' : ":\call sy#jump#prev_hunk(v:count1)\"
nnoremap <silent> <expr> <Plug>(signify-next-hunk) &diff ? ']c' : ":\call sy#jump#next_hunk(v:count1)\"
vnoremap <M-l> :Limelight!!
nnoremap <M-l> :Limelight!!
vnoremap <M-Space> :Goyo
nnoremap <M-Space> :Goyo
nnoremap <M-CR> :vsplit | terminal
nnoremap <F6> :call vimspector#Launch()
nnoremap <F5> :make!
nnoremap <Left> :call vimspector#Restart()
nnoremap <Up> :call vimspector#StepOut()
nnoremap <Right> :call vimspector#StepOver()
nnoremap <Down> :call vimspector#StepInto()
nmap <M-e> :CocCommand explorer
nnoremap <silent> <M-9> 9gt
nnoremap <silent> <M-8> 8gt
nnoremap <silent> <M-7> 7gt
nnoremap <silent> <M-6> 6gt
nnoremap <silent> <M-5> 5gt
nnoremap <silent> <M-4> 4gt
nnoremap <silent> <M-3> 3gt
nnoremap <silent> <M-2> 2gt
nnoremap <silent> <M-1> 1gt
nnoremap <silent> <M-.> :silent! exec "tabn". g:lasttab
imap S <Plug>ISurround
imap s <Plug>Isurround
imap  <Plug>(UnicodeFuzzy)
imap <NL> <M-n>a
imap  <Plug>(coc-snippets-expand)
imap  <Plug>Isurround
imap  <Plug>(HTMLEntityComplete)
imap  <Plug>(UnicodeComplete)
imap  <Plug>(DigraphComplete)
inoremap  :Dict
inoremap  :ThesaurusQueryReplaceCurrentWorda
inoreabbr hh &
inoreabbr #! #!/usr/bin/bash
inoreabbr @@ johann123j@gmail.com
let &cpo=s:cpo_save
unlet s:cpo_save
set autowrite
set cindent
set clipboard=unnamedplus
set cmdheight=2
set completefunc=thesaurus_query#auto_complete_integrate
set copyindent
set cpoptions=aABceFs_Il
set errorfile=/tmp/nvimsmENqL/94
set expandtab
set helplang=ja
set hidden
set ignorecase
set iskeyword=a-z,A-Z,48-57,_,-,ä,ö,ü,ß
set lazyredraw
set mouse=a
set operatorfunc=<SNR>30_go
set pastetoggle=<F2>
set pyxversion=3
set runtimepath=~/.config/nvim,~/.config/nvim/bundle/vim-indent-object,~/.config/nvim/bundle/vim-signify,~/.config/nvim/bundle/julia-vim,~/.config/nvim/bundle/vim-livedown,~/.config/nvim/bundle/vimspector,~/.config/nvim/bundle/lightline.vim,~/.config/nvim/bundle/vim-commentary,~/.config/nvim/bundle/vimtex,~/.config/nvim/bundle/limelight.vim,~/.config/nvim/bundle/unicode.vim,~/.config/nvim/bundle/goyo.vim,~/.config/nvim/bundle/vim-surround,~/.config/nvim/bundle/coc.nvim,~/.config/nvim/bundle/tex-conceal.vim,~/.config/nvim/bundle/vim-fugitive,~/.config/nvim/bundle/gruvbox,~/.config/nvim/bundle/vim-projectroot,~/.config/nvim/bundle/vim-dispatch,~/.config/nvim/bundle/quick-scope,~/.config/nvim/bundle/vim-css-color,~/.config/nvim/bundle/vim-gutentags,~/.config/nvim/bundle/thesaurus_query.vim,~/.config/nvim/bundle/vim-dict,~/.config/nvim/bundle/auto-pairs,~/.config/nvim/bundle/ctrlp.vim,~/.config/nvim/bundle/targets.vim,~/.config/nvim/bundle/vim-ripgrep,~/.config/nvim/bundle/vim-octave,~/.config/coc/extensions/node_modules/coc-todolist,/etc/xdg/nvim,~/.local/share/nvim/site,/usr/local/share/nvim/site,/usr/share/nvim/site,/var/lib/snapd/desktop/nvim/site,/usr/share/nvim/runtime,/usr/share/nvim/runtime/pack/dist/opt/matchit,/var/lib/snapd/desktop/nvim/site/after,/usr/share/nvim/site/after,/usr/local/share/nvim/site/after,~/.local/share/nvim/site/after,/etc/xdg/nvim/after,~/.config/nvim/after,~/.config/coc/extensions/node_modules/coc-snippets,~/.config/coc/extensions/node_modules/coc-explorer,~/.config/nvim/bundle/vimtex/after,~/.config/nvim/bundle/tex-conceal.vim/after,~/.config/nvim/bundle/vim-css-color/after,/usr/share/vim/vimfiles
set scrolloff=3
set shiftwidth=4
set shortmess=filnxtToOFcI
set smartcase
set splitright
set noswapfile
set synmaxcol=500
set tabline=%!lightline#tabline()
set tabstop=4
set termguicolors
set undodir=~/.config/nvim/undodir
set undofile
set undolevels=10000
set updatetime=300
set viewoptions=cursor
set virtualedit=onemore
set wildignore=*.odt,*.pdf,*.xopp,*.hi,*.o,*.pyc
set window=31
" vim: set ft=vim :
