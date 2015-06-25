#ifndef ROSE_RSIM_Linux32_H 
#define ROSE_RSIM_Linux32_H

#include "RSIM_Linux.h"

/** Simulator for 32-bit Linux guests. */
class RSIM_Linux32: public RSIM_Linux {
    std::vector<uint32_t> auxv_;                       // auxv vector pushed onto initial stack; also used when dumping core
public:
    RSIM_Linux32() {
        init();
    }

    virtual bool isSupportedArch(SgAsmGenericHeader*) ROSE_OVERRIDE;
    virtual void loadSpecimenNative(RSIM_Process*, rose::BinaryAnalysis::Disassembler*) ROSE_OVERRIDE;
    virtual void initializeSimulatedOs(RSIM_Process*, SgAsmGenericHeader*) ROSE_OVERRIDE;
    virtual PtRegs initialRegistersArch() ROSE_OVERRIDE;
    virtual rose_addr_t pushAuxVector(RSIM_Process*, rose_addr_t sp, rose_addr_t execfn_va, SgAsmElfFileHeader*,
                                      FILE *trace) ROSE_OVERRIDE;

private:
    void init();

protected:
    // System calls specific to 32-bit Linux
    static void syscall_alarm_body(RSIM_Thread *t, int callno);
    static void syscall_alarm_enter(RSIM_Thread *t, int callno);
    static void syscall_chdir_body(RSIM_Thread *t, int callno);
    static void syscall_chdir_enter(RSIM_Thread *t, int callno);
    static void syscall_chmod_body(RSIM_Thread *t, int callno);
    static void syscall_chmod_enter(RSIM_Thread *t, int callno);
    static void syscall_chown_body(RSIM_Thread *t, int callno);
    static void syscall_chown_enter(RSIM_Thread *t, int callno);
    static void syscall_clock_getres_body(RSIM_Thread *t, int callno);
    static void syscall_clock_getres_enter(RSIM_Thread *t, int callno);
    static void syscall_clock_getres_leave(RSIM_Thread *t, int callno);
    static void syscall_clock_gettime_body(RSIM_Thread *t, int callno);
    static void syscall_clock_gettime_enter(RSIM_Thread *t, int callno);
    static void syscall_clock_gettime_leave(RSIM_Thread *t, int callno);
    static void syscall_clock_settime_body(RSIM_Thread *t, int callno);
    static void syscall_clock_settime_enter(RSIM_Thread *t, int callno);
    static void syscall_clone_body(RSIM_Thread *t, int callno);
    static void syscall_clone_enter(RSIM_Thread *t, int callno);
    static void syscall_clone_leave(RSIM_Thread *t, int callno);
    static void syscall_execve_body(RSIM_Thread *t, int callno);
    static void syscall_execve_enter(RSIM_Thread *t, int callno);
    static void syscall_fchdir_body(RSIM_Thread *t, int callno);
    static void syscall_fchdir_enter(RSIM_Thread *t, int callno);
    static void syscall_fchmod_body(RSIM_Thread *t, int callno);
    static void syscall_fchmod_enter(RSIM_Thread *t, int callno);
    static void syscall_fchmodat_body(RSIM_Thread *t, int callno);
    static void syscall_fchmodat_enter(RSIM_Thread *t, int callno);
    static void syscall_fchown_body(RSIM_Thread *t, int callno);
    static void syscall_fchown_enter(RSIM_Thread *t, int callno);
    static void syscall_fchown32_body(RSIM_Thread *t, int callno);
    static void syscall_fchown32_enter(RSIM_Thread *t, int callno);
    static void syscall_fcntl_body(RSIM_Thread *t, int callno);
    static void syscall_fcntl_enter(RSIM_Thread *t, int callno);
    static void syscall_fcntl_leave(RSIM_Thread *t, int callno);
    static void syscall_fstatfs_body(RSIM_Thread *t, int callno);
    static void syscall_fstatfs_enter(RSIM_Thread *t, int callno);
    static void syscall_fstatfs_leave(RSIM_Thread *t, int callno);
    static void syscall_fstatfs64_body(RSIM_Thread *t, int callno);
    static void syscall_fstatfs64_enter(RSIM_Thread *t, int callno);
    static void syscall_fstatfs64_leave(RSIM_Thread *t, int callno);
    static void syscall_fsync_body(RSIM_Thread *t, int callno);
    static void syscall_fsync_enter(RSIM_Thread *t, int callno);
    static void syscall_ftruncate_body(RSIM_Thread *t, int callno);
    static void syscall_ftruncate_enter(RSIM_Thread *t, int callno);
    static void syscall_futex_body(RSIM_Thread *t, int callno);
    static void syscall_futex_enter(RSIM_Thread *t, int callno);
    static void syscall_futex_leave(RSIM_Thread *t, int callno);
    static void syscall_getcwd_body(RSIM_Thread *t, int callno);
    static void syscall_getcwd_enter(RSIM_Thread *t, int callno);
    static void syscall_getcwd_leave(RSIM_Thread *t, int callno);
    static void syscall_getdents_body(RSIM_Thread *t, int callno);
    static void syscall_getdents_enter(RSIM_Thread *t, int callno);
    static void syscall_getdents_leave(RSIM_Thread *t, int callno);
    static void syscall_getdents64_body(RSIM_Thread *t, int callno);
    static void syscall_getdents64_enter(RSIM_Thread *t, int callno);
    static void syscall_getdents64_leave(RSIM_Thread *t, int callno);
    static void syscall_getegid_body(RSIM_Thread *t, int callno);
    static void syscall_getegid_enter(RSIM_Thread *t, int callno);
    static void syscall_getegid32_body(RSIM_Thread *t, int callno);
    static void syscall_getegid32_enter(RSIM_Thread *t, int callno);
    static void syscall_geteuid_body(RSIM_Thread *t, int callno);
    static void syscall_geteuid_enter(RSIM_Thread *t, int callno);
    static void syscall_geteuid32_body(RSIM_Thread *t, int callno);
    static void syscall_geteuid32_enter(RSIM_Thread *t, int callno);
    static void syscall_getgid_body(RSIM_Thread *t, int callno);
    static void syscall_getgid_enter(RSIM_Thread *t, int callno);
    static void syscall_getgid32_body(RSIM_Thread *t, int callno);
    static void syscall_getgid32_enter(RSIM_Thread *t, int callno);
    static void syscall_getgroups32_body(RSIM_Thread *t, int callno);
    static void syscall_getgroups32_enter(RSIM_Thread *t, int callno);
    static void syscall_getgroups32_leave(RSIM_Thread *t, int callno);
    static void syscall_getpgrp_body(RSIM_Thread *t, int callno);
    static void syscall_getpgrp_enter(RSIM_Thread *t, int callno);
    static void syscall_getpid_body(RSIM_Thread *t, int callno);
    static void syscall_getpid_enter(RSIM_Thread *t, int callno);
    static void syscall_getppid_body(RSIM_Thread *t, int callno);
    static void syscall_getppid_enter(RSIM_Thread *t, int callno);
    static void syscall_getrlimit_body(RSIM_Thread *t, int callno);
    static void syscall_getrlimit_enter(RSIM_Thread *t, int callno);
    static void syscall_getrlimit_leave(RSIM_Thread *t, int callno);
    static void syscall_gettid_body(RSIM_Thread *t, int callno);
    static void syscall_gettid_enter(RSIM_Thread *t, int callno);
    static void syscall_gettimeofday_body(RSIM_Thread *t, int callno);
    static void syscall_gettimeofday_enter(RSIM_Thread *t, int callno);
    static void syscall_gettimeofday_leave(RSIM_Thread *t, int callno);
    static void syscall_getuid_body(RSIM_Thread *t, int callno);
    static void syscall_getuid_enter(RSIM_Thread *t, int callno);
    static void syscall_getuid32_body(RSIM_Thread *t, int callno);
    static void syscall_getuid32_enter(RSIM_Thread *t, int callno);
    static void syscall_ioctl_body(RSIM_Thread *t, int callno);
    static void syscall_ioctl_enter(RSIM_Thread *t, int callno);
    static void syscall_ioctl_leave(RSIM_Thread *t, int callno);
    static void syscall_ipc_body(RSIM_Thread *t, int callno);
    static void syscall_ipc_enter(RSIM_Thread *t, int callno);
    static void syscall_ipc_leave(RSIM_Thread *t, int callno);
    static void syscall_kill_body(RSIM_Thread *t, int callno);
    static void syscall_kill_enter(RSIM_Thread *t, int callno);
    static void syscall_link_body(RSIM_Thread *t, int callno);
    static void syscall_link_enter(RSIM_Thread *t, int callno);
    static void syscall_llseek_body(RSIM_Thread *t, int callno);
    static void syscall_llseek_enter(RSIM_Thread *t, int callno);
    static void syscall_lseek_body(RSIM_Thread *t, int callno);
    static void syscall_lseek_enter(RSIM_Thread *t, int callno);
    static void syscall_madvise_body(RSIM_Thread *t, int callno);
    static void syscall_madvise_enter(RSIM_Thread *t, int callno);
    static void syscall_mkdir_body(RSIM_Thread *t, int callno);
    static void syscall_mkdir_enter(RSIM_Thread *t, int callno);
    static void syscall_mknod_body(RSIM_Thread *t, int callno);
    static void syscall_mknod_enter(RSIM_Thread *t, int callno);
    static void syscall_mmap_body(RSIM_Thread *t, int callno);
    static void syscall_mmap_enter(RSIM_Thread *t, int callno);
    static void syscall_mmap_leave(RSIM_Thread *t, int callno);
    static void syscall_mmap2_body(RSIM_Thread *t, int callno);
    static void syscall_mmap2_enter(RSIM_Thread *t, int callno);
    static void syscall_mmap2_leave(RSIM_Thread *t, int callno);
    static void syscall_modify_ldt_body(RSIM_Thread *t, int callno);
    static void syscall_modify_ldt_enter(RSIM_Thread *t, int callno);
    static void syscall_modify_ldt_leave(RSIM_Thread *t, int callno);
    static void syscall_msync_body(RSIM_Thread *t, int callno);
    static void syscall_msync_enter(RSIM_Thread *t, int callno);
    static void syscall_nanosleep_body(RSIM_Thread *t, int callno);
    static void syscall_nanosleep_enter(RSIM_Thread *t, int callno);
    static void syscall_nanosleep_leave(RSIM_Thread *t, int callno);
    static void syscall_pause_body(RSIM_Thread *t, int callno);
    static void syscall_pause_enter(RSIM_Thread *t, int callno);
    static void syscall_pause_leave(RSIM_Thread *t, int callno);
    static void syscall_prctl_body(RSIM_Thread *t, int callno);
    static void syscall_prctl_enter(RSIM_Thread *t, int callno);
    static void syscall_pread64_body(RSIM_Thread *t, int callno);
    static void syscall_pread64_enter(RSIM_Thread *t, int callno);
    static void syscall_pread64_leave(RSIM_Thread *t, int callno);
    static void syscall_readlink_body(RSIM_Thread *t, int callno);
    static void syscall_readlink_enter(RSIM_Thread *t, int callno);
    static void syscall_rename_body(RSIM_Thread *t, int callno);
    static void syscall_rename_enter(RSIM_Thread *t, int callno);
    static void syscall_rmdir_body(RSIM_Thread *t, int callno);
    static void syscall_rmdir_enter(RSIM_Thread *t, int callno);
    static void syscall_rt_sigaction_body(RSIM_Thread *t, int callno);
    static void syscall_rt_sigaction_enter(RSIM_Thread *t, int callno);
    static void syscall_rt_sigaction_leave(RSIM_Thread *t, int callno);
    static void syscall_rt_sigpending_body(RSIM_Thread *t, int callno);
    static void syscall_rt_sigpending_enter(RSIM_Thread *t, int callno);
    static void syscall_rt_sigpending_leave(RSIM_Thread *t, int callno);
    static void syscall_rt_sigprocmask_body(RSIM_Thread *t, int callno);
    static void syscall_rt_sigprocmask_enter(RSIM_Thread *t, int callno);
    static void syscall_rt_sigprocmask_leave(RSIM_Thread *t, int callno);
    static void syscall_rt_sigreturn_body(RSIM_Thread *t, int callno);
    static void syscall_rt_sigreturn_enter(RSIM_Thread *t, int callno);
    static void syscall_rt_sigreturn_leave(RSIM_Thread *t, int callno);
    static void syscall_rt_sigsuspend_body(RSIM_Thread *t, int callno);
    static void syscall_rt_sigsuspend_enter(RSIM_Thread *t, int callno);
    static void syscall_rt_sigsuspend_leave(RSIM_Thread *t, int callno);
    static void syscall_sched_get_priority_max_body(RSIM_Thread *t, int callno);
    static void syscall_sched_get_priority_max_enter(RSIM_Thread *t, int callno);
    static void syscall_sched_get_priority_min_body(RSIM_Thread *t, int callno);
    static void syscall_sched_get_priority_min_enter(RSIM_Thread *t, int callno);
    static void syscall_sched_getaffinity_body(RSIM_Thread *t, int callno);
    static void syscall_sched_getaffinity_enter(RSIM_Thread *t, int callno);
    static void syscall_sched_getaffinity_leave(RSIM_Thread *t, int callno);
    static void syscall_sched_getscheduler_body(RSIM_Thread *t, int callno);
    static void syscall_sched_getscheduler_enter(RSIM_Thread *t, int callno);
    static void syscall_sched_getscheduler_leave(RSIM_Thread *t, int callno);
    static void syscall_sched_setparam_body(RSIM_Thread *t, int callno);
    static void syscall_sched_setparam_enter(RSIM_Thread *t, int callno);
    static void syscall_sched_setscheduler_body(RSIM_Thread *t, int callno);
    static void syscall_sched_setscheduler_enter(RSIM_Thread *t, int callno);
    static void syscall_sched_yield_body(RSIM_Thread *t, int callno);
    static void syscall_sched_yield_enter(RSIM_Thread *t, int callno);
    static void syscall_select_body(RSIM_Thread *t, int callno);
    static void syscall_select_enter(RSIM_Thread *t, int callno);
    static void syscall_select_leave(RSIM_Thread *t, int callno);
    static void syscall_setgroups32_body(RSIM_Thread *t, int callno);
    static void syscall_setgroups32_enter(RSIM_Thread *t, int callno);
    static void syscall_set_robust_list_body(RSIM_Thread *t, int callno);
    static void syscall_set_robust_list_enter(RSIM_Thread *t, int callno);
    static void syscall_set_thread_area_body(RSIM_Thread *t, int callno);
    static void syscall_set_thread_area_enter(RSIM_Thread *t, int callno);
    static void syscall_set_thread_area_leave(RSIM_Thread *t, int callno);
    static void syscall_set_tid_address_body(RSIM_Thread *t, int callno);
    static void syscall_set_tid_address_enter(RSIM_Thread *t, int callno);
    static void syscall_setpgid_body(RSIM_Thread *t, int callno);
    static void syscall_setpgid_enter(RSIM_Thread *t, int callno);
    static void syscall_setrlimit_body(RSIM_Thread *t, int callno);
    static void syscall_setrlimit_enter(RSIM_Thread *t, int callno);
    static void syscall_sigaltstack_body(RSIM_Thread *t, int callno);
    static void syscall_sigaltstack_enter(RSIM_Thread *t, int callno);
    static void syscall_sigaltstack_leave(RSIM_Thread *t, int callno);
    static void syscall_sigreturn_body(RSIM_Thread *t, int callno);
    static void syscall_sigreturn_enter(RSIM_Thread *t, int callno);
    static void syscall_sigreturn_leave(RSIM_Thread *t, int callno);
    static void syscall_socketcall_body(RSIM_Thread *t, int callno);
    static void syscall_socketcall_enter(RSIM_Thread *t, int callno);
    static void syscall_socketcall_leave(RSIM_Thread *t, int callno);
    static void syscall_stat64_body(RSIM_Thread *t, int callno);
    static void syscall_stat64_enter(RSIM_Thread *t, int callno);
    static void syscall_stat64_leave(RSIM_Thread *t, int callno);
    static void syscall_statfs_body(RSIM_Thread *t, int callno);
    static void syscall_statfs_enter(RSIM_Thread *t, int callno);
    static void syscall_statfs_leave(RSIM_Thread *t, int callno);
    static void syscall_statfs64_body(RSIM_Thread *t, int callno);
    static void syscall_statfs64_enter(RSIM_Thread *t, int callno);
    static void syscall_statfs64_leave(RSIM_Thread *t, int callno);
    static void syscall_symlink_body(RSIM_Thread *t, int callno);
    static void syscall_symlink_enter(RSIM_Thread *t, int callno);
    static void syscall_sync_body(RSIM_Thread *t, int callno);
    static void syscall_sync_enter(RSIM_Thread *t, int callno);
    static void syscall_sysinfo_body(RSIM_Thread *t, int callno);
    static void syscall_sysinfo_enter(RSIM_Thread *t, int callno);
    static void syscall_tgkill_body(RSIM_Thread *t, int callno);
    static void syscall_tgkill_enter(RSIM_Thread *t, int callno);
    static void syscall_time_body(RSIM_Thread *t, int callno);
    static void syscall_time_enter(RSIM_Thread *t, int callno);
    static void syscall_time_leave(RSIM_Thread *t, int callno);
    static void syscall_ugetrlimit_body(RSIM_Thread *t, int callno);
    static void syscall_ugetrlimit_enter(RSIM_Thread *t, int callno);
    static void syscall_ugetrlimit_leave(RSIM_Thread *t, int callno);
    static void syscall_umask_body(RSIM_Thread *t, int callno);
    static void syscall_umask_enter(RSIM_Thread *t, int callno);
    static void syscall_uname_body(RSIM_Thread *t, int callno);
    static void syscall_uname_enter(RSIM_Thread *t, int callno);
    static void syscall_uname_leave(RSIM_Thread *t, int callno);
    static void syscall_unlink_body(RSIM_Thread *t, int callno);
    static void syscall_unlink_enter(RSIM_Thread *t, int callno);
    static void syscall_utime_body(RSIM_Thread *t, int callno);
    static void syscall_utime_enter(RSIM_Thread *t, int callno);
    static void syscall_utimes_body(RSIM_Thread *t, int callno);
    static void syscall_utimes_enter(RSIM_Thread *t, int callno);
    static void syscall_wait4_body(RSIM_Thread *t, int callno);
    static void syscall_wait4_enter(RSIM_Thread *t, int callno);
    static void syscall_wait4_leave(RSIM_Thread *t, int callno);
    static void syscall_waitpid_body(RSIM_Thread *t, int callno);
    static void syscall_waitpid_enter(RSIM_Thread *t, int callno);
    static void syscall_waitpid_leave(RSIM_Thread *t, int callno);
    static void syscall_writev_body(RSIM_Thread *t, int callno);
    static void syscall_writev_enter(RSIM_Thread *t, int callno);

};

#endif /* ROSE_RSIM_Linux32_H */




