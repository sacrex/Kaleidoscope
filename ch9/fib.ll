; ModuleID = 'my cool jit'
source_filename = "my cool jit"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

declare double @printd(double)

define double @"binary:"(double %x, double %y) !dbg !4 {
entry:
  %y2 = alloca double
  %x1 = alloca double
  call void @llvm.dbg.declare(metadata double* %x1, metadata !9, metadata !DIExpression()), !dbg !11
  store double %x, double* %x1
  call void @llvm.dbg.declare(metadata double* %y2, metadata !10, metadata !DIExpression()), !dbg !11
  store double %y, double* %y2
  %y3 = load double, double* %y2, !dbg !12
  ret double %y3, !dbg !12
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

define double @fib(double %x) !dbg !13 {
entry:
  %x1 = alloca double
  call void @llvm.dbg.declare(metadata double* %x1, metadata !17, metadata !DIExpression()), !dbg !18
  store double %x, double* %x1
  %x2 = load double, double* %x1, !dbg !19
  %cmptmp = fcmp ult double %x2, 3.000000e+00, !dbg !20
  %booltmp = uitofp i1 %cmptmp to double, !dbg !20
  %ifcond = fcmp one double %booltmp, 0.000000e+00, !dbg !20
  br i1 %ifcond, label %then, label %else, !dbg !20

then:                                             ; preds = %entry
  br label %ifcont, !dbg !21

else:                                             ; preds = %entry
  %x3 = load double, double* %x1, !dbg !22
  %subtmp = fsub double %x3, 1.000000e+00, !dbg !23
  %calltmp = call double @fib(double %subtmp), !dbg !23
  %x4 = load double, double* %x1, !dbg !24
  %subtmp5 = fsub double %x4, 2.000000e+00, !dbg !25
  %calltmp6 = call double @fib(double %subtmp5), !dbg !25
  %addtmp = fadd double %calltmp, %calltmp6, !dbg !25
  br label %ifcont, !dbg !25

ifcont:                                           ; preds = %else, %then
  %iftmp = phi double [ 1.000000e+00, %then ], [ %addtmp, %else ], !dbg !25
  ret double %iftmp, !dbg !25
}

define void @main() !dbg !26 {
entry:
  %i = alloca double
  store double 1.000000e+00, double* %i, !dbg !29
  br label %loopend, !dbg !29

loopend:                                          ; preds = %loop, %entry
  %i1 = load double, double* %i, !dbg !30
  %cmptmp = fcmp ult double %i1, 2.000000e+01, !dbg !31
  %booltmp = uitofp i1 %cmptmp to double, !dbg !31
  %loopcond = fcmp one double %booltmp, 0.000000e+00, !dbg !31
  br i1 %loopcond, label %loop, label %afterloop, !dbg !31

loop:                                             ; preds = %loopend
  %i2 = load double, double* %i, !dbg !32
  %calltmp = call double @fib(double %i2), !dbg !32
  %calltmp3 = call double @printd(double %calltmp), !dbg !32
  %i4 = load double, double* %i, !dbg !32
  %nextvar = fadd double %i4, 1.000000e+00, !dbg !32
  store double %nextvar, double* %i, !dbg !32
  br label %loopend, !dbg !32

afterloop:                                        ; preds = %loopend
  ret void, !dbg !32
}

attributes #0 = { nounwind readnone speculatable }

!llvm.module.flags = !{!0}
!llvm.dbg.cu = !{!1}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = distinct !DICompileUnit(language: DW_LANG_C, file: !2, producer: "Kaleidoscope Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !3)
!2 = !DIFile(filename: "fib.ka", directory: ".")
!3 = !{}
!4 = distinct !DISubprogram(name: "binary:", scope: !2, file: !2, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !7, !7}
!7 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!8 = !{!9, !10}
!9 = !DILocalVariable(name: "x", arg: 1, scope: !4, file: !2, line: 3, type: !7)
!10 = !DILocalVariable(name: "y", arg: 2, scope: !4, file: !2, line: 3, type: !7)
!11 = !DILocation(line: 3, scope: !4)
!12 = !DILocation(line: 3, column: 22, scope: !4)
!13 = distinct !DISubprogram(name: "fib", scope: !2, file: !2, line: 5, type: !14, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !16)
!14 = !DISubroutineType(types: !15)
!15 = !{!7, !7}
!16 = !{!17}
!17 = !DILocalVariable(name: "x", arg: 1, scope: !13, file: !2, line: 5, type: !7)
!18 = !DILocation(line: 5, scope: !13)
!19 = !DILocation(line: 6, column: 5, scope: !13)
!20 = !DILocation(line: 6, column: 9, scope: !13)
!21 = !DILocation(line: 7, column: 3, scope: !13)
!22 = !DILocation(line: 9, column: 7, scope: !13)
!23 = !DILocation(line: 9, column: 11, scope: !13)
!24 = !DILocation(line: 9, column: 20, scope: !13)
!25 = !DILocation(line: 9, column: 24, scope: !13)
!26 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 11, type: !27, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !3)
!27 = !DISubroutineType(types: !28)
!28 = !{!7}
!29 = !DILocation(line: 11, column: 9, scope: !26)
!30 = !DILocation(line: 11, column: 12, scope: !26)
!31 = !DILocation(line: 11, column: 16, scope: !26)
!32 = !DILocation(line: 12, column: 13, scope: !26)
