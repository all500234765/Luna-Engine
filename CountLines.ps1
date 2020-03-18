# Old
#(gci -include *.cpp,*.h,*.hlsl,*.hlsli -exclude */Vendor/*,*/Compiled/*,*/Shaders/* -recurse | select-string .).Count

# Idk why, but -exclude doesn't work. Tried on several PCs

$lines = 0
$vars  = 0

$direct = @('Windows','AVC Example','Audio Engine','Physics Engine','Shaders','EngineIncludes','DirectX11 Engine 2019')

$reg = "^[ ]*(?(?=(?!return|template|struct|private|public|protected|case|default|if|switch|delete|0x|[0-9]+|typedef|using|break|while|for|\[[a-zA-Z\(\)0-9]\]|enum|class|friend|#|\/|\||\n))\s*(?!:)([a-zA-Z0-9_]\s*[*&:]*(\<([a-zA-Z0-9_, *+\/-])*\>)*\s*[*&, \n]*)+[^{]$|(SKIP)^)"
#"^[ ]*(?!return|template|struct|private|public|protected|case|default|if|switch|delete|0x|[0-9]+|typedef|using|break|while|for|\[[a-zA-Z\(\)0-9]\]|enum|class|friend|#|\/|\||\n)\s*(?!:)([a-zA-Z0-9_]\s*[*&:]*(\<([a-zA-Z0-9_, *+\/-])*\>)*\s*[*&, \n]*)+[^{]$"
#"([a-zA-Z_]+[a-zA-Z0-9_]*(\<[a-zA-Z0-9_]+\>)*[(\:\:) ]+[a-zA-Z_]+[a-zA-Z0-9_]*)+([() ])*;"

# Loop through all directories
for( $i = 0; $i -lt $direct.length; $i++ ) {
    cd $direct[$i]
        # Find amount of lines in all files in current sub-dir
        $lines_local = (dir -include *.h,*.cpp,*.hlsl,*.hlsli -recurse | select-string "^(\s*)//" -notMatch | select-string "^(\s*)$" -notMatch).Count
        
		$output_file = "..\" + $direct[$i] + ".txt"
		
		# | % { $_.Matches } | % { $_.Value } > $output_file
		# Find amount of variables in all files in current sub-dir
		$vars_local = (dir -include *.h,*.cpp,*.hlsl,*.hlsli -recurse | select-string -Pattern $reg -AllMatches).Count
        
		$vars = $vars + $vars_local
        $lines = $lines + $lines_local
        
        '+-----------------------------------'
        '|In sub project: ' + $direct[$i]
        '| Lines: ' + $lines_local
		'| Vars: ' + $vars_local
        '+-----------------------------------'
        ''
		
    cd ..
}

'' 
'+------------------------------------'
'|Total counters in core engine projects'
'| Lines: ' + $lines
'| Vars: ' + $vars
'+------------------------------------'

