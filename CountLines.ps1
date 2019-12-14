# Old
#(gci -include *.cpp,*.h,*.hlsl,*.hlsli -exclude */Vendor/*,*/Compiled/*,*/Shaders/* -recurse | select-string .).Count

# Idk why, but -exclude doesn't work. Tried on several PCs

$lines = 0

$direct = @('DirectX11 Engine 2019','Audio Engine','Physics Engine','Shaders','EngineIncludes','Windows','AVC Example')

# Loop through all directories
for( $i = 0; $i -lt $direct.length; $i++ ) {
    cd $direct[$i]
        # Find amount of lines in all files in current sub-dir
        $lines_local = (dir -include *.h,*.cpp,*.hlsl,*.hlsli -recurse | select-string "^(\s*)//" -notMatch | select-string "^(\s*)$" -notMatch).Count
        
        $lines = $lines + $lines_local
        
        '+-----------------------------------'
        '|In sub project: ' + $direct[$i]
        '|' + $lines_local
        '+-----------------------------------'
        ''
    cd ..
}

'' 
'+------------------------------------'
'|Total number of lines in the core engine projects'
'|' + $lines
'+------------------------------------'

