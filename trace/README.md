### Build & Run

```
# build TraceGenPass, life_game.ll and life_game
mkdir build && cd build
cmake ..
make

# save trace of the most representable part of execution
./life_game | head -100000000 | tail -1000000 > trace.txt

# analyze trace
perl ../analyze.pl < trace.txt --mode=instr
perl ../analyze.pl < trace.txt --mode=bb -n 10
```


### Results
Basic blocks:
```
FREQUENCY  PATTERN
34.15%     ; function 'main' BB '%"10"'
34.15%     ; function 'main' BB '%"22"'
27.93%     ; function 'update_map' BB '%"184"'
3.00%      ; function 'main' BB '%"15"'
0.17%      ; function 'main' BB '%"7"'
0.17%      ; function 'main' BB '%"4"'
0.14%      ; function 'update_map' BB '%"183"'
0.14%      ; function 'update_map' BB '%"1"'
0.14%      ; function 'update_map' BB '%"174"'
0.00%      ; function 'update_map' BB '%"220"'
0.00%      ; function 'main' BB '%"25"'
0.00%      ; function 'main' BB '%"3"'
0.00%      ; function 'update_map' BB '%"0"'

============================================

; function 'main' BB '%"10"'
%"12" = getelementptr inbounds %struct.map_t, %struct.map_t* %"1", i64 0, i32 0, i64 %"5", i64 %"11"
%"13" = load i8, i8* %"12", align 1, !tbaa !7
%"14" = icmp eq i8 %"13", 0
br i1 %"14", label %"22", label %"15"
; function 'main' BB '%"22"'
%"23" = add nuw nsw i64 %"11", 1
%"24" = icmp eq i64 %"23", 200
br i1 %"24", label %"7", label %"10", !llvm.loop !12

; function 'main' BB '%"22"'
%"23" = add nuw nsw i64 %"11", 1
%"24" = icmp eq i64 %"23", 200
br i1 %"24", label %"7", label %"10", !llvm.loop !12
; function 'main' BB '%"10"'
%"12" = getelementptr inbounds %struct.map_t, %struct.map_t* %"1", i64 0, i32 0, i64 %"5", i64 %"11"
%"13" = load i8, i8* %"12", align 1, !tbaa !7
%"14" = icmp eq i8 %"13", 0
br i1 %"14", label %"22", label %"15"

; function 'update_map' BB '%"184"'
%"186" = trunc i64 %"185" to i16
%"187" = add i16 %"186", 199
%"188" = urem i16 %"187", 200
%"189" = add nuw nsw i64 %"185", 1
%"190" = icmp ult i64 %"185", 199
%"191" = add nuw nsw i64 %"185", 57
%"192" = select i1 %"190", i64 %"189", i64 %"191"
%"193" = and i64 %"192", 255
%"194" = zext i16 %"188" to i64
%"195" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"181", i64 %"194"
%"196" = load i8, i8* %"195", align 1, !tbaa !5
%"197" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"181", i64 %"185"
%"198" = load i8, i8* %"197", align 1, !tbaa !5
%"199" = add i8 %"198", %"196"
%"200" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"181", i64 %"193"
%"201" = load i8, i8* %"200", align 1, !tbaa !5
%"202" = add i8 %"199", %"201"
%"203" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"175", i64 %"194"
%"204" = load i8, i8* %"203", align 1, !tbaa !5
%"205" = add i8 %"202", %"204"
%"206" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"175", i64 %"193"
%"207" = load i8, i8* %"206", align 1, !tbaa !5
%"208" = add i8 %"205", %"207"
%"209" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"182", i64 %"194"
%"210" = load i8, i8* %"209", align 1, !tbaa !5
%"211" = add i8 %"208", %"210"
%"212" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"182", i64 %"185"
%"213" = load i8, i8* %"212", align 1, !tbaa !5
%"214" = add i8 %"211", %"213"
%"215" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"182", i64 %"193"
%"216" = load i8, i8* %"215", align 1, !tbaa !5
%"217" = add i8 %"214", %"216"
%"218" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %"175", i64 %"185"
store i8 %"217", i8* %"218", align 1, !tbaa !5
%"219" = icmp eq i64 %"189", 200
br i1 %"219", label %"183", label %"184", !llvm.loop !11
```

Instructions:
```
FREQUENCY  PATTERN
2.63%      %"23" = add nuw nsw i64 %"11", 1
2.63%      %"24" = icmp eq i64 %"23", 200
2.63%      br i1 %"24", label %"7", label %"10", !llvm.loop !12
2.63%      %"12" = getelementptr inbounds %struct.map_t, %struct.map_t* %"1", i64 0, i32 0, i64 %"5", i64 %"11"
2.63%      br i1 %"14", label %"22", label %"15"
2.63%      %"14" = icmp eq i8 %"13", 0
2.63%      %"13" = load i8, i8* %"12", align 1, !tbaa !7
2.15%      %"207" = load i8, i8* %"206", align 1, !tbaa !5
2.15%      %"219" = icmp eq i64 %"189", 200
2.15%      %"210" = load i8, i8* %"209", align 1, !tbaa !5
2.15%      %"199" = add i8 %"198", %"196"
2.15%      %"195" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"181", i64 %"194"
2.15%      %"196" = load i8, i8* %"195", align 1, !tbaa !5
2.15%      %"218" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %"175", i64 %"185"
2.15%      %"208" = add i8 %"205", %"207"
2.15%      %"191" = add nuw nsw i64 %"185", 57
2.15%      %"197" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"181", i64 %"185"
2.15%      %"198" = load i8, i8* %"197", align 1, !tbaa !5
2.15%      %"204" = load i8, i8* %"203", align 1, !tbaa !5
2.15%      %"189" = add nuw nsw i64 %"185", 1
2.15%      %"206" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"175", i64 %"193"
2.15%      %"192" = select i1 %"190", i64 %"189", i64 %"191"
2.15%      %"190" = icmp ult i64 %"185", 199
2.15%      %"194" = zext i16 %"188" to i64
2.15%      %"202" = add i8 %"199", %"201"
2.15%      %"212" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"182", i64 %"185"
2.15%      %"188" = urem i16 %"187", 200
2.15%      %"216" = load i8, i8* %"215", align 1, !tbaa !5
2.15%      store i8 %"217", i8* %"218", align 1, !tbaa !5
2.15%      %"187" = add i16 %"186", 199
2.15%      %"215" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"182", i64 %"193"
2.15%      %"193" = and i64 %"192", 255
2.15%      %"211" = add i8 %"208", %"210"
2.15%      %"217" = add i8 %"214", %"216"
2.15%      %"209" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"182", i64 %"194"
2.15%      %"203" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"175", i64 %"194"
2.15%      %"200" = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %"181", i64 %"193"
2.15%      %"205" = add i8 %"202", %"204"
2.15%      %"214" = add i8 %"211", %"213"
2.15%      %"186" = trunc i64 %"185" to i16
2.15%      %"201" = load i8, i8* %"200", align 1, !tbaa !5
2.15%      br i1 %"219", label %"183", label %"184", !llvm.loop !11
2.15%      %"213" = load i8, i8* %"212", align 1, !tbaa !5
0.23%      br label %"22"
0.23%      %"19" = getelementptr inbounds [9 x i32], [9 x i32]* @__const.draw_map.color_scheme, i64 0, i64 %"18"
...
```

### Analysis
Frequent patterns:
- load + add, load + cmp (probably need cisc-like arithmetic instructions)
- icmp + branch
- trunc, zext, urem
