
$output_file = ".\TestOut.txt"
$f = Get-Content ".\Test.txt"
$reg = "^[ ]*(?(?=(?!return|template|struct|private|public|protected|case|default|if|switch|delete|0x|[0-9]+|typedef|using|break|while|for|\[[a-zA-Z\(\)0-9]\]|enum|class|friend|#|\/|\||\n))\s*(?!:)([a-zA-Z0-9_]\s*[*&:]*(\<([a-zA-Z0-9_, *+\/-])*\>)*\s*[*&, \n]*)+[^{]$|(*SKIP)^)"
#"^[ ]*(?!return|template|struct|private|public|protected|case|default|if|switch|delete|0x|[0-9]+|typedef|using|break|while|for|\[[a-zA-Z\(\)0-9]\]|enum|class|friend|#|\/|\||\n)\s*(?!:)([a-zA-Z0-9_]\s*[*&:]*(\<([a-zA-Z0-9_, *+\/-])*\>)*\s*[*&, \n]*)+[^{]$"
#"([a-zA-Z_]+[a-zA-Z0-9_]*(\<[a-zA-Z0-9_]+\>)*[(\:\:) ]+[a-zA-Z_]+[a-zA-Z0-9_]*)+([() ])*;"
#'f = ' + $f

cd "Windows"
$q = (gc (dir -include *.h,*.cpp,*.hlsl,*.hlsli -recurse))

'$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$'
$q

# | % { $_.Matches } | % { $_.Value } > $output_file
# 
$c = ($q | select-string -Pattern $reg -AllMatches) #($f | select-string -Pattern $reg -AllMatches).Count
cd ..

'$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$'
'c = ' + $c

'$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$'
'count = ' + $c.Count

