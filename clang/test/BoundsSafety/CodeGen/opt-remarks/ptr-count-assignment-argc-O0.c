// XFAIL: *

// RUN: %clang_cc1 -triple x86_64-apple-macos -fbounds-safety -O0 %s -emit-llvm -o %t-O0.s -opt-record-file %t-O0.opt.yaml -opt-record-format yaml
// RUN: FileCheck --input-file %t-O0.s --check-prefixes IR %s
// RUN: FileCheck --input-file %t-O0.opt.yaml --check-prefixes OPT-REM %s

#include <ptrcheck.h>

int main(int argc, char **argv) {
    int count_rt = argc;
    int * __counted_by(count_rt) buff_rt = 0;
    count_rt = argc;
    buff_rt = buff_rt;
}

// IR: define{{.*}} i32 @main(i32 noundef %argc, ptr noundef %argv)
// IR: store i32 %argc, ptr %[[ARGC_ALLOCA:[a-z0-9.]+]]
// IR: %[[ARGC_LOAD1:[a-z0-9.]+]] = load i32, ptr %[[ARGC_ALLOCA]], align 4, !dbg ![[LOC_10_20:[0-9]+]]
// IR: store i32 %[[ARGC_LOAD1]], ptr %[[COUNT_RT_ALLOCA:[a-z0-9._]+]], align 4, !dbg ![[LOC_10_9:[0-9]+]]
// IR: %[[COUNT_RT_LOAD:[a-z0-9._]+]] = load i32, ptr %[[COUNT_RT_ALLOCA]], align 4, !dbg ![[LOC_12_16:[0-9]+]]
// IR: icmp eq i32 %[[COUNT_RT_LOAD]], 0, !dbg ![[LOC_11_44:[0-9]+]], !annotation ![[NEW_COUNT_POSITIVE:[a-z0-9]+]]
// IR: br i1 {{.*}}, label %[[LABEL_CONT:cont]], label %[[LABEL_TRAP:trap]], !dbg ![[LOC_11_44]], !annotation ![[NEW_COUNT_POSITIVE]]
// ...
// IR: [[LABEL_TRAP]]:
// IR: call void @llvm.ubsantrap(i8 25) #{{[0-9]+}}, !dbg ![[TRAP_LOC_11_44:[0-9]+]], !annotation ![[NEW_COUNT_POSITIVE]]
// IR-NEXT: unreachable, !dbg ![[TRAP_LOC_11_44]], !annotation ![[NEW_COUNT_POSITIVE]]
// ...
// IR: [[LABEL_CONT]]:
// ...
// IR: %[[ARGC_LOAD2:[a-z0-9.]+]] = load i32, ptr %[[ARGC_ALLOCA]], align 4, !dbg ![[LOC_12_16:[0-9]+]]
// IR: icmp ule {{.*}} !dbg ![[LOC_12_14:[0-9]+]]
// IR: br i1 %{{.*}}, label %[[MAIN_LABEL_CONT:.+]], label %[[MAIN_LABEL_TRAP_RES:.+]], !dbg ![[LOC_12_14]]

// IR: [[MAIN_LABEL_CONT]]:
// ...
// IR: icmp ule {{.*}} !dbg ![[LOC_12_14]]
// IR: br i1 %{{.*}}, label %[[MAIN_LABEL_CONT2:.+]], label %[[MAIN_LABEL_TRAP_RES]], !dbg ![[LOC_12_5:[0-9]+]]

// IR: [[MAIN_LABEL_CONT2]]:
// ...
// IR: %[[WIDTH_CHECK_RES:[a-z0-9_]+]] = icmp sle {{.*}} !dbg ![[LOC_12_5]]
// IR: br i1 %{{.*}}, label %[[MAIN_LABEL_EMPTY:.+]], label %[[MAIN_LABEL_TRAP_RES2:.*]], !dbg ![[LOC_12_5]]

// IR: [[MAIN_LABEL_EMPTY]]:
// IR: %[[ARGC_CMP:[a-z0-9_]+]] = icmp sle i32 0, %[[ARGC_LOAD2]], !dbg ![[LOC_12_5]]
// IR: br label %[[MAIN_LABEL_TRAP_RES2]]

// IR: [[MAIN_LABEL_TRAP_RES2]]:
// IR: %[[TRAP_RES2:[a-z0-9_]+]] = phi i1 [ false, %[[MAIN_LABEL_CONT2]] ], [ %[[ARGC_CMP]], %[[MAIN_LABEL_EMPTY]] ], !dbg ![[TRAP_LOC_MISSING:[0-9]+]]
// IR: br label %[[MAIN_LABEL_TRAP_RES]]

// IR: [[MAIN_LABEL_TRAP_RES]]:
// IR: %[[TRAP_RES:[a-z0-9_]+]] = phi i1 [ false, %[[MAIN_LABEL_CONT]] ], [ false, %[[LABEL_CONT]] ], [ %[[TRAP_RES2]], %[[MAIN_LABEL_TRAP_RES2]] ], !dbg ![[TRAP_LOC_MISSING:[0-9]+]], !annotation ![[ANNOT_CONV_TO_COUNT:[0-9]+]]
// IR: br i1 %[[TRAP_RES]], label {{.*}}, label %[[MAIN_LABEL_TRAP:[a-z0-9.]+]], !dbg ![[LOC_12_5]], !annotation ![[ANNOT_CONV_TO_COUNT]]

// IR: [[MAIN_LABEL_TRAP]]:
// IR: call void @llvm.ubsantrap(i8 25) #{{[0-9]+}}, !dbg ![[TRAP_LOC_12_14:[0-9]+]], !annotation ![[ANNOT_CONV_TO_COUNT]]
// IR-NEXT: unreachable, !dbg ![[TRAP_LOC_12_14]], !annotation ![[ANNOT_CONV_TO_COUNT]]

// IR-DAG: ![[LOC_10_9]] = !DILocation(line: 10, column: 9
// IR-DAG: ![[LOC_10_20]] = !DILocation(line: 10, column: 20
// IR-DAG: ![[LOC_11_44]] = !DILocation(line: 11, column: 44
// IR-DAG: ![[LOC_12_5]] = !DILocation(line: 12, column: 5
// IR-DAG: ![[LOC_12_16]] = !DILocation(line: 12, column: 16

// IR-DAG: ![[LOC_12_14]] = !DILocation(line: 12, column: 14
// IR-DAG: ![[TRAP_LOC_12_14]] = !DILocation(line: 0, scope: ![[TRAP_INFO_BNDS_CHECK_FAILED:[0-9]+]], inlinedAt: ![[LOC_12_5]])
// IR-DAG: ![[TRAP_INFO_BNDS_CHECK_FAILED]] = distinct !DISubprogram(name: "__clang_trap_msg$Bounds check failed$"
// IR-DAG: ![[TRAP_LOC_11_44]] = !DILocation(line: 0, scope: ![[TRAP_INFO_BNDS_CHECK_FAILED]], inlinedAt: ![[LOC_11_44]])

// IR-DAG: ![[TRAP_LOC_MISSING]] = !DILocation(line: 0, scope: ![[MAIN_SCOPE:[0-9]+]])
// IR-DAG: ![[MAIN_SCOPE]] = distinct !DISubprogram(name: "main", {{.*}} line: 9, {{.*}} scopeLine: 9

// IR-DAG: ![[NEW_COUNT_POSITIVE]] = !{!"bounds-safety-generic"}

// opt-remarks tests generated using `gen-opt-remarks-check-lines.py`

// OPT-REM: --- !Analysis
// OPT-REM-NEXT: Pass:            annotation-remarks
// OPT-REM-NEXT: Name:            AnnotationSummary
// OPT-REM-NEXT: DebugLoc:        { File: '{{.*}}ptr-count-assignment-argc-O0.c',
// OPT-REM-NEXT:                    Line: 9, Column: 0 }
// OPT-REM-NEXT: Function:        main
// OPT-REM-NEXT: Args:
// OPT-REM-NEXT:   - String:          'Annotated '
// OPT-REM-NEXT:   - count:           '53'
// OPT-REM-NEXT:   - String:          ' instructions with '
// OPT-REM-NEXT:   - type:            bounds-safety-generic
// OPT-REM-NEXT: ...

// OPT-REM-NEXT: --- !Analysis
// OPT-REM-NEXT: Pass:            annotation-remarks
// OPT-REM-NEXT: Name:            AnnotationSummary
// OPT-REM-NEXT: DebugLoc:        { File: '{{.*}}ptr-count-assignment-argc-O0.c',
// OPT-REM-NEXT:                    Line: 9, Column: 0 }
// OPT-REM-NEXT: Function:        main
// OPT-REM-NEXT: Args:
// OPT-REM-NEXT:   - String:          'Annotated '
// OPT-REM-NEXT:   - count:           '53'
// OPT-REM-NEXT:   - String:          ' instructions with '
// OPT-REM-NEXT:   - type:            bounds-safety-total-summary
// OPT-REM-NEXT: ...

// OPT-REM-NEXT: --- !Analysis
// OPT-REM-NEXT: Pass:            annotation-remarks
// OPT-REM-NEXT: Name:            BoundsSafetyCheck
// OPT-REM-NEXT: DebugLoc:        { File: '{{.*}}ptr-count-assignment-argc-O0.c',
// OPT-REM-NEXT:                    Line: 11, Column: 11 }
// OPT-REM-NEXT: Function:        main
// OPT-REM-NEXT: Args:
// OPT-REM-NEXT:   - String:          'Inserted '
// OPT-REM-NEXT:   - count:           '1'
// OPT-REM-NEXT:   - String:          ' LLVM IR instruction'
// OPT-REM-NEXT:   - String:          ''
// OPT-REM-NEXT:   - String:          "\n"
// OPT-REM-NEXT:   - String:          "used for:\n"
// OPT-REM-NEXT:   - String:          bounds-safety-generic
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT:       instructions:
// OPT-REM-EMPTY:
// OPT-REM-NEXT:   - String:          'other (LLVM IR ''load'')'
// OPT-REM-NEXT: ...

// OPT-REM-NEXT: --- !Analysis
// OPT-REM-NEXT: Pass:            annotation-remarks
// OPT-REM-NEXT: Name:            BoundsSafetyCheck
// OPT-REM-NEXT: DebugLoc:        { File: '{{.*}}ptr-count-assignment-argc-O0.c',
// OPT-REM-NEXT:                    Line: 11, Column: 44 }
// OPT-REM-NEXT: Function:        main
// OPT-REM-NEXT: Args:
// OPT-REM-NEXT:   - String:          'Inserted '
// OPT-REM-NEXT:   - count:           '2'
// OPT-REM-NEXT:   - String:          ' LLVM IR instruction'
// OPT-REM-NEXT:   - String:          s
// OPT-REM-NEXT:   - String:          "\n"
// OPT-REM-NEXT:   - String:          "used for:\n"
// OPT-REM-NEXT:   - String:          bounds-safety-generic
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT:       instructions:
// OPT-REM-EMPTY:
// OPT-REM-NEXT:   - String:          "cmp eq (LLVM IR 'icmp')\ncond branch (LLVM IR 'br')"
// OPT-REM-NEXT: ...

// OPT-REM-NEXT: --- !Analysis
// OPT-REM-NEXT: Pass:            annotation-remarks
// OPT-REM-NEXT: Name:            BoundsSafetyCheck
// OPT-REM-NEXT: DebugLoc:        { File: '{{.*}}ptr-count-assignment-argc-O0.c',
// OPT-REM-NEXT:                    Line: 0, Column: 0 }
// OPT-REM-NEXT: Function:        main
// OPT-REM-NEXT: Args:
// OPT-REM-NEXT:   - String:          'Inserted '
// OPT-REM-NEXT:   - count:           '2'
// OPT-REM-NEXT:   - String:          ' LLVM IR instruction'
// OPT-REM-NEXT:   - String:          s
// OPT-REM-NEXT:   - String:          "\n"
// OPT-REM-NEXT:   - String:          "used for:\n"
// OPT-REM-NEXT:   - String:          bounds-safety-generic
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT:       instructions:
// OPT-REM-EMPTY:
// OPT-REM-NEXT:   - String:          "trap (LLVM IR 'call')\nother (LLVM IR 'unreachable')"
// OPT-REM-NEXT: ...

// OPT-REM-NEXT: --- !Analysis
// OPT-REM-NEXT: Pass:            annotation-remarks
// OPT-REM-NEXT: Name:            BoundsSafetyCheck
// OPT-REM-NEXT: DebugLoc:        { File: '{{.*}}ptr-count-assignment-argc-O0.c',
// OPT-REM-NEXT:                    Line: 13, Column: 15 }
// OPT-REM-NEXT: Function:        main
// OPT-REM-NEXT: Args:
// OPT-REM-NEXT:   - String:          'Inserted '
// OPT-REM-NEXT:   - count:           '24'
// OPT-REM-NEXT:   - String:          ' LLVM IR instruction'
// OPT-REM-NEXT:   - String:          s
// OPT-REM-NEXT:   - String:          "\n"
// OPT-REM-NEXT:   - String:          "used for:\n"
// OPT-REM-NEXT:   - String:          bounds-safety-generic
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT:       instructions:
// OPT-REM-EMPTY:
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT:       call (LLVM IR 'call')
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-NEXT:       call (LLVM IR 'call')
// OPT-REM-NEXT:       call (LLVM IR 'call')
// OPT-REM-NEXT:       call (LLVM IR 'call')
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-NEXT:       call (LLVM IR 'call')
// OPT-REM-NEXT:       call (LLVM IR 'call')
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-EMPTY:
// OPT-REM-NEXT: ...

// OPT-REM-NEXT: --- !Analysis
// OPT-REM-NEXT: Pass:            annotation-remarks
// OPT-REM-NEXT: Name:            BoundsSafetyCheck
// OPT-REM-NEXT: DebugLoc:        { File: '{{.*}}ptr-count-assignment-argc-O0.c',
// OPT-REM-NEXT:                    Line: 12, Column: 5 }
// OPT-REM-NEXT: Function:        main
// OPT-REM-NEXT: Args:
// OPT-REM-NEXT:   - String:          'Inserted '
// OPT-REM-NEXT:   - count:           '15'
// OPT-REM-NEXT:   - String:          ' LLVM IR instruction'
// OPT-REM-NEXT:   - String:          s
// OPT-REM-NEXT:   - String:          "\n"
// OPT-REM-NEXT:   - String:          "used for:\n"
// OPT-REM-NEXT:   - String:          bounds-safety-generic
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT:       instructions:
// OPT-REM-EMPTY:
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-NEXT:       cond branch (LLVM IR 'br')
// OPT-REM-NEXT:       other (LLVM IR 'getelementptr')
// OPT-REM-NEXT:       other (LLVM IR 'load')
// OPT-REM-NEXT:       other (LLVM IR 'ptrtoint')
// OPT-REM-NEXT:       other (LLVM IR 'ptrtoint')
// OPT-REM-NEXT:       other (LLVM IR 'sub')
// OPT-REM-NEXT:       other (LLVM IR 'sdiv')
// OPT-REM-NEXT:       cmp sle (LLVM IR 'icmp')
// OPT-REM-NEXT:       cond branch (LLVM IR 'br')
// OPT-REM-NEXT:       cmp sle (LLVM IR 'icmp')
// OPT-REM-NEXT:       cond branch (LLVM IR 'br')
// OPT-REM-EMPTY:
// OPT-REM-NEXT: ...

// OPT-REM-NEXT: --- !Analysis
// OPT-REM-NEXT: Pass:            annotation-remarks
// OPT-REM-NEXT: Name:            BoundsSafetyCheck
// OPT-REM-NEXT: DebugLoc:        { File: '{{.*}}ptr-count-assignment-argc-O0.c',
// OPT-REM-NEXT:                    Line: 12, Column: 14 }
// OPT-REM-NEXT: Function:        main
// OPT-REM-NEXT: Args:
// OPT-REM-NEXT:   - String:          'Inserted '
// OPT-REM-NEXT:   - count:           '3'
// OPT-REM-NEXT:   - String:          ' LLVM IR instruction'
// OPT-REM-NEXT:   - String:          s
// OPT-REM-NEXT:   - String:          "\n"
// OPT-REM-NEXT:   - String:          "used for:\n"
// OPT-REM-NEXT:   - String:          bounds-safety-generic
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT:       instructions:
// OPT-REM-EMPTY:
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT:       cmp ule (LLVM IR 'icmp')
// OPT-REM-NEXT:       cond branch (LLVM IR 'br')
// OPT-REM-NEXT:       cmp ule (LLVM IR 'icmp')
// OPT-REM-EMPTY:
// OPT-REM-NEXT: ...

// OPT-REM-NEXT: --- !Analysis
// OPT-REM-NEXT: Pass:            annotation-remarks
// OPT-REM-NEXT: Name:            BoundsSafetyCheck
// OPT-REM-NEXT: DebugLoc:        { File: '{{.*}}ptr-count-assignment-argc-O0.c',
// OPT-REM-NEXT:                    Line: 12, Column: 16 }
// OPT-REM-NEXT: Function:        main
// OPT-REM-NEXT: Args:
// OPT-REM-NEXT:   - String:          'Inserted '
// OPT-REM-NEXT:   - count:           '1'
// OPT-REM-NEXT:   - String:          ' LLVM IR instruction'
// OPT-REM-NEXT:   - String:          ''
// OPT-REM-NEXT:   - String:          "\n"
// OPT-REM-NEXT:   - String:          "used for:\n"
// OPT-REM-NEXT:   - String:          bounds-safety-generic
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT:       instructions:
// OPT-REM-EMPTY:
// OPT-REM-NEXT:   - String:          'other (LLVM IR ''sext'')'
// OPT-REM-NEXT: ...

// OPT-REM-NEXT: --- !Analysis
// OPT-REM-NEXT: Pass:            annotation-remarks
// OPT-REM-NEXT: Name:            BoundsSafetyCheck
// OPT-REM-NEXT: DebugLoc:        { File: '{{.*}}ptr-count-assignment-argc-O0.c',
// OPT-REM-NEXT:                    Line: 0, Column: 0 }
// OPT-REM-NEXT: Function:        main
// OPT-REM-NEXT: Args:
// OPT-REM-NEXT:   - String:          'Inserted '
// OPT-REM-NEXT:   - count:           '1'
// OPT-REM-NEXT:   - String:          ' LLVM IR instruction'
// OPT-REM-NEXT:   - String:          ''
// OPT-REM-NEXT:   - String:          "\n"
// OPT-REM-NEXT:   - String:          "used for:\n"
// OPT-REM-NEXT:   - String:          bounds-safety-generic
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT:       instructions:
// OPT-REM-EMPTY:
// OPT-REM-NEXT:   - String:          'other (LLVM IR ''phi'')'
// OPT-REM-NEXT: ...

// OPT-REM-NEXT: --- !Analysis
// OPT-REM-NEXT: Pass:            annotation-remarks
// OPT-REM-NEXT: Name:            BoundsSafetyCheck
// OPT-REM-NEXT: DebugLoc:        { File: '{{.*}}ptr-count-assignment-argc-O0.c',
// OPT-REM-NEXT:                    Line: 0, Column: 0 }
// OPT-REM-NEXT: Function:        main
// OPT-REM-NEXT: Args:
// OPT-REM-NEXT:   - String:          'Inserted '
// OPT-REM-NEXT:   - count:           '2'
// OPT-REM-NEXT:   - String:          ' LLVM IR instruction'
// OPT-REM-NEXT:   - String:          s
// OPT-REM-NEXT:   - String:          "\n"
// OPT-REM-NEXT:   - String:          "used for:\n"
// OPT-REM-NEXT:   - String:          bounds-safety-generic
// OPT-REM-NEXT:   - String:           |
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT: {{^[ 	]+$}}
// OPT-REM-NEXT:       instructions:
// OPT-REM-EMPTY:
// OPT-REM-NEXT:   - String:          "trap (LLVM IR 'call')\nother (LLVM IR 'unreachable')"
// OPT-REM-NEXT: ...

// OPT-REM-NOT: --- !Analysis
