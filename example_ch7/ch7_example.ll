; ModuleID = 'ch7_example.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G = common global i32 0, align 4
@H = common global i32 0, align 4

; Function Attrs: nounwind uwtable
define i32 @test(i1 zeroext %Condition) #0 {
  %1 = alloca i8, align 1
  %X = alloca i32, align 4
  %2 = zext i1 %Condition to i8
  store i8 %2, i8* %1, align 1
  %3 = load i8, i8* %1, align 1
  %4 = trunc i8 %3 to i1
  br i1 %4, label %5, label %7

; <label>:5                                       ; preds = %0
  %6 = load i32, i32* @G, align 4
  store i32 %6, i32* %X, align 4
  br label %9

; <label>:7                                       ; preds = %0
  %8 = load i32, i32* @H, align 4
  store i32 %8, i32* %X, align 4
  br label %9

; <label>:9                                       ; preds = %7, %5
  %10 = load i32, i32* %X, align 4
  ret i32 %10
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0-2ubuntu3~trusty5 (tags/RELEASE_380/final)"}
