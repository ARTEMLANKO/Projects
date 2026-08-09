package info.kgeorgiy.ja.lanko.iterative;

import info.kgeorgiy.java.advanced.mapper.ParallelMapper;

import java.util.List;
import java.util.function.Function;

public class StructForTask<T, R> implements Runnable {
    public boolean passed;
    private Function<? super T, ? extends R> function;
    private T elementForFunction;
    public R result;
    public RuntimeException exception;
    public StructForTask(Function<? super T, ? extends R> function,
                         T elementForFunction) {
        this.function = function;
        this.elementForFunction = elementForFunction;
        passed = false;
    }

    @Override
    public void run() {
        try {
            result = function.apply(elementForFunction);
            if (Thread.currentThread().isInterrupted()) {
                exception = new IllegalStateException("PM was closed");
            }
        } catch (RuntimeException e) {
            exception = e;
        } finally {
            synchronized (this) {
                passed = true;
                notifyAll();
            }
        }
    }
}