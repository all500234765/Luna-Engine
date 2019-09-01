

(gci -include *.cpp,*.h,*.hlsl,*.hlsli -exclude */Vendor/*,*/Compiled/*,*/Shaders/* -recurse | select-string .).Count
