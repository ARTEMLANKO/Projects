package info.kgeorgiy.ja.lanko.iterative;

import info.kgeorgiy.java.advanced.iterative.AdvancedIP;
import info.kgeorgiy.java.advanced.mapper.ParallelMapper;

import java.util.*;
import java.util.function.*;
import java.util.stream.IntStream;

public class IterativeParallelism implements AdvancedIP {
    private ParallelMapper mapper;

    public IterativeParallelism() {}

    public IterativeParallelism(final ParallelMapper mapper) {
        this.mapper = mapper;
    }

    private void checkErrors(final List<RuntimeException> errors) {
        RuntimeException ex = null;
        for (final RuntimeException exp : errors) {
            if (ex == null) {
                ex = exp;
            } else {
                ex.addSuppressed(exp);
            }
        }
        if (ex != null) {
            throw ex;
        }
    }

    private class MyPair {
        private final int start;
        private final int end;

        public MyPair(int start, int end) {
            this.start = start;
            this.end = end;
        }

        public int getStart() {
            return start;
        }

        public int getEnd() {
            return end;
        }
    }

    private <T, E, R> R general(
            int threadCount, final List<T> list, final int step, final BiFunction<Integer, Integer, E> task,
            final Function<List<E>, R> resultFunction
    ) throws InterruptedException {
        threadCount = Math.min(threadCount, list.size());
        if (mapper != null) {
            final List<MyPair> indexes = new ArrayList<>();
            for (int i = 0; i < threadCount; i++) {
                final MyPair tmpInd = index(list.size(), threadCount, i);
                final int indStart = tmpInd.getStart() + (step - tmpInd.getStart() % step) % step;
                final int indEnd = tmpInd.getEnd();
                indexes.add(new MyPair(indStart, indEnd));
            }
            final List<E> result = mapper.map((MyPair x) -> task.apply(x.getStart(), x.getEnd()),
                    indexes);
            return resultFunction.apply(result);
        }
        final List<Thread> th = new ArrayList<>(threadCount);
        final List<E> results = new ArrayList<>(threadCount);
        final List<RuntimeException> errors = new ArrayList<>(threadCount);
        for (int i = 0; i < threadCount; i++) {
            th.add(null);
            results.add(null);
            errors.add(null);
        }


        for (int i = 0; i < th.size(); i++) {
            final int threadIndex = i;
            final MyPair indexes = index(list.size(), threadCount, threadIndex);
            final int indStart = indexes.getStart() + (step - indexes.getStart() % step) % step;
            final int indEnd = indexes.getEnd();
            th.set(i, new Thread(() -> {
                try {
                    results.set(threadIndex, task.apply(indStart, indEnd));
                } catch (final RuntimeException e) {
                    errors.set(threadIndex, e);
                }
            }));
        }
        threadsAction(th);
        checkErrors(errors);
        return resultFunction.apply(results);
    }

    public <T> int[] indices(final int threadCount, final List<? extends T> list, final Predicate<? super T> predicate, final int step) throws InterruptedException {
        return general(threadCount, list, step, ((indStart, indEnd) ->
                        IntStream.iterate(indStart, i -> i < indEnd, i -> i + step)
                                .filter(i -> predicate.test(list.get(i)))
                                .toArray()),
                (results) -> results.stream().flatMapToInt(Arrays::stream).toArray()
        );
    }

    public <T> List<T> filter(final int threadCount, final List<? extends T> list, final Predicate<? super T> predicate, final int step) throws InterruptedException {
        return general(threadCount, list, step, ((indStart, indEnd) ->
                        IntStream.iterate(indStart, i -> i < indEnd, i -> i + step)
                                .filter(i -> predicate.test(list.get(i)))
                                .<T>mapToObj(list::get).toList()
                ),
                (results) -> results.stream().flatMap(Collection::stream).toList()
        );
    }

    public <T, R> List<R> map(final int threadCount, final List<? extends T> list, final Function<? super T, ? extends R> function, final int step) throws InterruptedException {
        if (function == null) {
            throw new NullPointerException("function is null");
        }
        return general(threadCount, list, step, ((indStart, indEnd) ->
                        IntStream.iterate(indStart, i -> i < indEnd, i -> i + step)
                                .<R>mapToObj(i -> function.apply(list.get(i))).toList()
                ),
                (results) -> results.stream().flatMap(Collection::stream).toList()
        );
    }

    public <T> int argMax(final int threadCount, final List<T> list, final Comparator<? super T> comparator, final int step) throws InterruptedException {
        return general(threadCount, list, step, ((indStart, indEnd) ->
                        IntStream.iterate(indStart, i -> i < indEnd, i -> i + step)
                                .reduce((i, j) -> Objects.compare(list.get(i), list.get(j), comparator) < 0 ? j : i)
                                .orElse(-1)
                ),
                (results) -> results.stream().reduce(-1, (ans, current) ->
                        (ans == -1 || current != -1 && Objects.compare(list.get(ans), list.get(current), comparator) < 0) ? current : ans));
    }

    public <T> int argMin(final int threadCount, final List<T> list, final Comparator<? super T> comparator, final int step) throws InterruptedException {
        return general(threadCount, list, step, ((indStart, indEnd) ->
                        IntStream.iterate(indStart, i -> i < indEnd, i -> i + step)
                                .reduce((i, j) -> Objects.compare(list.get(i), list.get(j), comparator) <= 0 ? i : j)
                                .orElse(-1)
                ),
                (results) -> results.stream().reduce(-1, (ans, current) ->
                        (ans == -1 || current != -1 && Objects.compare(list.get(ans), list.get(current), comparator) > 0) ? current : ans));
    }

    public <T> int indexOf(final int threadCount, final List<T> list, final Predicate<? super T> predicate, final int step) throws InterruptedException {
        return general(threadCount, list, step, ((indStart, indEnd) ->
                        IntStream.iterate(indStart, i -> i < indEnd, i -> i + step)
                                .filter(i -> predicate.test(list.get(i)))
                                .findFirst().orElse(-1)
                ),
                (results) -> results.stream().filter(x -> x != -1).findFirst().orElseGet(() -> -1));
    }

    public <T> int lastIndexOf(final int threadCount, final List<T> list, final Predicate<? super T> predicate, final int step) throws InterruptedException {
        return general(threadCount, list, step, ((indStart, indEnd) ->
                        IntStream.iterate(indStart, i -> i < indEnd, i -> i + step)
                                .filter(i -> predicate.test(list.get(i)))
                                .reduce((i, j) -> j)
                                .orElse(-1)
                ),
                (results) -> IntStream.range(0, results.size()).map(ind -> results.get(results.size() - ind - 1))
                        .filter(x -> x != -1).findFirst().orElseGet(() -> -1));
    }

    public <T> long sumIndices(final int threadCount, final List<? extends T> list, final Predicate<? super T> predicate, final int step) throws InterruptedException {
        return general(threadCount, list, step, ((indStart, indEnd) ->
                    IntStream.iterate(indStart, i -> i < indEnd, i -> i + step)
                            .filter(i -> predicate.test(list.get(i)))
                            .asLongStream().sum()
                ),
                (results) -> results.stream().reduce(0L, Long::sum));
    }

    private void threadsAction(final List<Thread> th) throws InterruptedException {
        for (final Thread thread : th) {
            thread.start();
        }
        InterruptedException exp = null;
        for (final Thread thread : th) {
            try {
                thread.join();
            } catch (final InterruptedException e) {
                if (exp == null) {
                    exp = e;
                } else {
                    exp.addSuppressed(e);
                }
                Thread.currentThread().interrupt();
            }
        }
        if (exp != null) {
            throw exp;
        }
    }

    public <T> T reduce(final int threadCount, final List<T> list, final T t, final BinaryOperator<T> binaryOperator, final int step) throws InterruptedException {
        return general(threadCount, list, step, ((indStart, indEnd) ->
                    IntStream.iterate(indStart, i -> i < indEnd, i -> i + step)
                            .mapToObj(list::get)
                            .reduce(t, binaryOperator)
                ),
                (results) -> results.stream().skip(1).reduce(results.getFirst(), binaryOperator));
    }

    public <T, R> R mapReduce(final int threadCount, final List<T> list, final Function<T, R> function, final R r, final BinaryOperator<R> binaryOperator, final int step) throws InterruptedException {
        return general(threadCount, list, step, ((indStart, indEnd) ->
                    IntStream.iterate(indStart, i -> i < indEnd, i -> i + step)
                            .mapToObj(list::get)
                            .map(function)
                            .reduce(r, binaryOperator)
                ),
                (results) -> results.stream().skip(1).reduce(results.getFirst(), binaryOperator));
    }

    private MyPair index(final int size, final int workers, final int i) {
        int tasksForWorker = size / workers;
        int remainingTasks = size % workers;
        int start;
        int end;
        if (remainingTasks == 0) {
            start = i * tasksForWorker;
            end = start + tasksForWorker;
        } else {
            start = Math.min(i, remainingTasks) * (1 + tasksForWorker) + Math.max(0, i - remainingTasks) * tasksForWorker;
            end = start + tasksForWorker;
            if (i <= remainingTasks - 1) {
                end++;
            }
        }
        return new MyPair(start, end);
    }

    @Override
    public <T> int argMax(final int threadCount, final List<T> list, final Comparator<? super T> comparator) throws InterruptedException {
        return argMax(threadCount, list, comparator, 1);

    }

    @Override
    public <T> int argMin(final int threadCount, final List<T> list, final Comparator<? super T> comparator) throws InterruptedException {
        return argMin(threadCount, list, comparator, 1);
    }

    @Override
    public <T> int indexOf(final int threadCount, final List<T> list, final Predicate<? super T> predicate) throws InterruptedException {
        return indexOf(threadCount, list, predicate, 1);
    }

    @Override
    public <T> int lastIndexOf(final int threadCount, final List<T> list, final Predicate<? super T> predicate) throws InterruptedException {
        return lastIndexOf(threadCount, list, predicate, 1);
    }

    @Override
    public <T> long sumIndices(final int threadCount, final List<? extends T> list, final Predicate<? super T> predicate) throws InterruptedException {
        return sumIndices(threadCount, list, predicate, 1);
    }

    @Override
    public <T> int[] indices(final int threadCount, final List<? extends T> list, final Predicate<? super T> predicate) throws InterruptedException {
        return indices(threadCount, list, predicate, 1);
    }

    @Override
    public <T> List<T> filter(final int threadCount, final List<? extends T> list, final Predicate<? super T> predicate) throws InterruptedException {
        return filter(threadCount, list, predicate, 1);
    }

    @Override
    public <T, R> List<R> map(final int threadCount, final List<? extends T> list, final Function<? super T, ? extends R> function) throws InterruptedException {
        return map(threadCount, list, function, 1);
    }
}
