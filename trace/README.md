### Build & Run

```bash
# build TraceGenPass, life.ll and life_game
mkdir build && cd build
cmake ..
make

# save trace of the most representative part of execution (size is ~90MB)
./life_game | head -100000000 | tail -2000000 > trace.txt

# analyze trace
perl ../analyze.pl < trace.txt --mode=instr
perl ../analyze.pl < trace.txt --mode=bb
```


### Results
Basic blocks:
```
FREQUENCY  PATTERN
32.15%     ; function 'update_map' BB '%185'
32.04%     ; function 'main' BB '%10'
32.04%     ; function 'main' BB '%22'
2.97%      ; function 'main' BB '%15'
0.16%      ; function 'main' BB '%4'
0.16%      ; function 'update_map' BB '%175'
0.16%      ; function 'update_map' BB '%184'
0.16%      ; function 'main' BB '%7'
0.16%      ; function 'update_map' BB '%2'
0.00%      ; function 'main' BB '%25'
0.00%      ; function 'update_map' BB '%221'
0.00%      ; function 'main' BB '%3'
0.00%      ; function 'update_map' BB '%1'

============================================

; function 'update_map' BB '%185'
%186 = phi i64 [ 0, %175 ], [ %190, %185 ]
%187 = trunc i64 %186 to i16
%188 = add i16 %187, 199
%189 = urem i16 %188, 200
%190 = add nuw nsw i64 %186, 1
%191 = icmp ult i64 %186, 199
%192 = add nuw nsw i64 %186, 57
%193 = select i1 %191, i64 %190, i64 %192
%194 = and i64 %193, 255
%195 = zext i16 %189 to i64
%196 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %182, i64 %195
%197 = load i8, i8* %196, align 1, !tbaa !5
%198 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %182, i64 %186
%199 = load i8, i8* %198, align 1, !tbaa !5
%200 = add i8 %199, %197
%201 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %182, i64 %194
%202 = load i8, i8* %201, align 1, !tbaa !5
%203 = add i8 %200, %202
%204 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %176, i64 %195
%205 = load i8, i8* %204, align 1, !tbaa !5
%206 = add i8 %203, %205
%207 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %176, i64 %194
%208 = load i8, i8* %207, align 1, !tbaa !5
%209 = add i8 %206, %208
%210 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %183, i64 %195
%211 = load i8, i8* %210, align 1, !tbaa !5
%212 = add i8 %209, %211
%213 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %183, i64 %186
%214 = load i8, i8* %213, align 1, !tbaa !5
%215 = add i8 %212, %214
%216 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %183, i64 %194
%217 = load i8, i8* %216, align 1, !tbaa !5
%218 = add i8 %215, %217
%219 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %176, i64 %186
store i8 %218, i8* %219, align 1, !tbaa !5
%220 = icmp eq i64 %190, 200
br i1 %220, label %184, label %185, !llvm.loop !11

; function 'main' BB '%10'
%11 = phi i64 [ 0, %4 ], [ %23, %22 ]
%12 = getelementptr inbounds %struct.map_t, %struct.map_t* %1, i64 0, i32 0, i64 %5, i64 %11
%13 = load i8, i8* %12, align 1, !tbaa !7
%14 = icmp eq i8 %13, 0
br i1 %14, label %22, label %15

; function 'main' BB '%22'
%23 = add nuw nsw i64 %11, 1
%24 = icmp eq i64 %23, 200
br i1 %24, label %7, label %10, !llvm.loop !12
```

Instructions:
```
FREQUENCY  PATTERN
2.14%      %203 = add i8 %200, %202
2.14%      %204 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %176, i64 %195
2.14%      %205 = load i8, i8* %204, align 1, !tbaa !5
2.14%      %201 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %182, i64 %194
2.14%      %202 = load i8, i8* %201, align 1, !tbaa !5
2.14%      %213 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %183, i64 %186
2.14%      %186 = phi i64 [ 0, %175 ], [ %190, %185 ]
2.14%      %200 = add i8 %199, %197
2.14%      %214 = load i8, i8* %213, align 1, !tbaa !5
2.14%      %195 = zext i16 %189 to i64
2.14%      %199 = load i8, i8* %198, align 1, !tbaa !5
2.14%      %207 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %176, i64 %194
2.14%      %218 = add i8 %215, %217
2.14%      %196 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %182, i64 %195
2.14%      %208 = load i8, i8* %207, align 1, !tbaa !5
2.14%      %187 = trunc i64 %186 to i16
2.14%      store i8 %218, i8* %219, align 1, !tbaa !5
2.14%      %192 = add nuw nsw i64 %186, 57
2.14%      %215 = add i8 %212, %214
2.14%      %211 = load i8, i8* %210, align 1, !tbaa !5
2.14%      %212 = add i8 %209, %211
2.14%      %198 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %182, i64 %186
2.14%      %194 = and i64 %193, 255
2.14%      %220 = icmp eq i64 %190, 200
2.14%      br i1 %220, label %184, label %185, !llvm.loop !11
2.14%      %206 = add i8 %203, %205
2.14%      %216 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %183, i64 %194
2.14%      %209 = add i8 %206, %208
2.14%      %197 = load i8, i8* %196, align 1, !tbaa !5
2.14%      %189 = urem i16 %188, 200
2.14%      %188 = add i16 %187, 199
2.14%      %219 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %176, i64 %186
2.14%      %190 = add nuw nsw i64 %186, 1
2.14%      %191 = icmp ult i64 %186, 199
2.14%      %217 = load i8, i8* %216, align 1, !tbaa !5
2.14%      %210 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %183, i64 %195
2.14%      %193 = select i1 %191, i64 %190, i64 %192
2.13%      %24 = icmp eq i64 %23, 200
2.13%      %11 = phi i64 [ 0, %4 ], [ %23, %22 ]
2.13%      %13 = load i8, i8* %12, align 1, !tbaa !7
2.13%      br i1 %14, label %22, label %15
2.13%      %23 = add nuw nsw i64 %11, 1
2.13%      %14 = icmp eq i8 %13, 0
2.13%      br i1 %24, label %7, label %10, !llvm.loop !12
2.13%      %12 = getelementptr inbounds %struct.map_t, %struct.map_t* %1, i64 0, i32 0, i64 %5, i64 %11
0.20%      %18 = zext i8 %17 to i64
...
```

### Analysis
1. very frequent load + add (can be optimized with a cisc-like instruction)
2. icmp eq + branch (also can be easily merged into one beq instruction)
3. frequent trunc, zext, urem, but without distinct usage pattern
