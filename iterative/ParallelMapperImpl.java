package info.kgeorgiy.ja.lanko.iterative;

import info.kgeorgiy.java.advanced.mapper.ParallelMapper;

import java.util.*;
import java.util.function.Function;


public class ParallelMapperImpl implements ParallelMapper {
    private final List<Thread> myThreads;
    protected volatile boolean isClosed;
    private final Deque<StructForTask<?, ?>> deque;
    private final Object objForSync;

    public ParallelMapperImpl(final int threads) {
        objForSync = new Object();
        synchronized (objForSync) {
            deque = new ArrayDeque<>();
            isClosed = false;
            myThreads = new ArrayList<>(Collections.nCopies(threads, null));
            for (int i = 0; i < threads; i++) {
                myThreads.set(i, new Thread(() -> {
                    while (!isClosed && !Thread.currentThread().isInterrupted()) {
                        StructForTask<?, ?> task;
                        try {
                            synchronized (deque) {
                                while (deque.isEmpty()) {
                                    if (isClosed) {
                                        return;
                                    }
                                    deque.wait();
                                }
                                if (isClosed) {
                                    return;
                                }
                                task = deque.removeFirst();
                            }
                        } catch (InterruptedException e) {
                            Thread.currentThread().interrupt();
                            return;
                        }
                        task.run();
                    }
                }));
            }
            for (int i = 0; i < threads; i++) {
                myThreads.get(i).start();
            }
        }
    }

    @Override
    public <T, R> List<R> map(Function<? super T, ? extends R> function, List<? extends T> list) throws InterruptedException {
        if (isClosed) {
            throw new IllegalStateException("PM is closed");
        }
        List<R> results = new ArrayList<>();
        List<StructForTask<T, R>> tasks = new ArrayList<>();
        for (int i = 0; i < list.size(); i++) {
            StructForTask<T, R> task = new StructForTask<>(function, list.get(i));
            tasks.add(task);
            synchronized (deque) {
                deque.add(task);
                deque.notifyAll();
            }
        }

        RuntimeException generalException = null;
        for (StructForTask<T, R> task : tasks) {
            try {
                synchronized (task) {
                    while (!task.passed) {
                        try {
                            task.wait();
                        } catch (InterruptedException e) {
                            Thread.currentThread().interrupt();
                            throw new RuntimeException(e);
                        }
                    }
                    if (task.exception != null) {
                        throw task.exception;
                    }
                    results.add(task.result);
                }
            } catch (RuntimeException e) {
                if (generalException == null) {
                    generalException = e;
                } else {
                    generalException.addSuppressed(e);
                }
            }
        }
        if (generalException != null) {
            throw generalException;
        }
        return results;
    }

    @Override
    public void close() {
        isClosed = true;
        synchronized (objForSync) {
            for (Thread th : myThreads) {
                th.interrupt();
                try {
                    th.join();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }
            for (StructForTask<?, ?> task : deque) {
                synchronized (task) {
                    task.passed = true;
                    task.notifyAll();
                }
            }
        }
    }
}
