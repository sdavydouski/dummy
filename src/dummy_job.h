#pragma once

struct job_queue;

#define JOB_ENTRY_POINT(name) void name(job_queue *Queue, void *Params)
typedef JOB_ENTRY_POINT(job_entry_point);

struct job
{
    job_entry_point *EntryPoint;
    void *Params;
    // todo(continue)
    memory_arena Arena;
};

struct job_queue
{
    // sync primitives
    void *Before;
    void *After;
    void *QueueNotEmpty;

    u32 volatile CurrentJobCount;
    i32 volatile CurrentJobIndex;

    job Jobs[256];
};

// Must be atomic!
inline job *
GetNextJobFromQueue(job_queue *JobQueue)
{
    job *Job = JobQueue->Jobs + JobQueue->CurrentJobIndex;

    JobQueue->CurrentJobIndex -= 1;

    Assert(JobQueue->CurrentJobIndex >= -1);

    return Job;
}

// This must be called only from master thread!
inline void
PutJobIntoQueue(job_queue *JobQueue, job *Job)
{
    JobQueue->CurrentJobIndex += 1;
    JobQueue->CurrentJobCount += 1;

    Assert(JobQueue->CurrentJobIndex < ArrayCount(JobQueue->Jobs));
    Assert(JobQueue->CurrentJobCount <= ArrayCount(JobQueue->Jobs));

    job *DestJob = &JobQueue->Jobs[JobQueue->CurrentJobIndex];
    DestJob->EntryPoint = Job->EntryPoint;
    DestJob->Params = Job->Params;
}

// This must be called only from master thread!
inline void
PutJobsIntoQueue(job_queue *JobQueue, u32 JobCount, job *Jobs)
{
    for (u32 JobIndex = 0; JobIndex < JobCount; ++JobIndex)
    {
        job *Job = Jobs + JobIndex;
        PutJobIntoQueue(JobQueue, Job);
    }
}
