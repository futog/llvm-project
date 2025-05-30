; RUN: opt -mtriple=x86_64-unknown-unknown -S -o - -passes=infer-address-spaces -assume-default-is-flat-addrspace %s | FileCheck %s

; Check that assert in X86TargetMachine::isNoopAddrSpaceCast is not triggered.

target datalayout = "e-p:64:64-p1:64:64-p2:32:32-p3:32:32-p4:64:64-p5:32:32-p6:32:32-p7:160:256:256:32-p8:128:128:128:48-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-v2048:2048-n32:64-S32-A5-ni:7:8"

; CHECK-LABEL: @noop_ptrint_pair(
; CHECK: addrspacecast ptr addrspace(1) %x to ptr addrspace(4)
; CHECK-NEXT: ptrtoint ptr addrspace(4) %{{.*}} to i64
; CHECK-NEXT: inttoptr i64 %{{.*}} to ptr addrspace(4)
define void @noop_ptrint_pair(ptr addrspace(1) %x) {
  %1 = addrspacecast ptr addrspace(1) %x to ptr addrspace(4)
  %2 = ptrtoint ptr addrspace(4) %1 to i64
  %3 = inttoptr i64 %2 to ptr addrspace(4)
  ret void
}
