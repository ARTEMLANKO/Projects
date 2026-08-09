package info.kgeorgiy.ja.lanko.crawler;

import info.kgeorgiy.java.advanced.crawler.*;

import java.io.IOException;
import java.net.MalformedURLException;
import java.util.*;
import java.util.concurrent.*;

@FunctionalInterface
interface MyBiPredicate {
    boolean apply(String s, List<String> list) throws MalformedURLException;
}

/**
 * download web-sites
 */
public class WebCrawler implements AdvancedCrawler {
    private Downloader downloader;
    private ExecutorService downloaders;
    private ExecutorService extractors;
    private int perHost;
    private ConcurrentHashMap<String, Semaphore> semaphores;

    /**
     *
     * @param downloader download sites
     * @param downloaders count of threads for download
     * @param extractors count of threads for links
     * @param perHost count of threads for one host
     */
    public WebCrawler(Downloader downloader, int downloaders, int extractors, int perHost) {
        this.downloader = downloader;
        this.downloaders = Executors.newFixedThreadPool(downloaders);
        this.extractors = Executors.newFixedThreadPool(extractors);
        this.perHost = perHost;
        this.semaphores = new ConcurrentHashMap<>();
    }

    /**
     *
     * @param args WebCrawler url [depth [downloaders [extractors [perHost]]]]
     * @throws IOException CachingDownload throw
     */
    public static void main(String[] args) throws IOException {
        String url = args[0];
        int depth = 10;
        if (args.length > 1) {
            depth = Integer.parseInt(args[1]);
        }
        int downloaders = 5;
        if (args.length > 2) {
            downloaders = Integer.parseInt(args[2]);
        }
        int extractors = 5;
        if (args.length > 3) {
            extractors = Integer.parseInt(args[3]);
        }
        int perHost = args.length > 4 ?  Integer.parseInt(args[4]) : 5;
        Downloader downloader = new CachingDownloader(10);
        WebCrawler webcrawler = new WebCrawler(downloader ,downloaders, extractors, perHost);
        Result ans = webcrawler.download(url, depth, new ArrayList<>());
        for (String s : ans.downloaded()) {
            System.out.println(s);
        }
    }

    /**
     *
     * @param s url
     * @param depth depth
     * @param list hosts, in which must be url
     * @return list of websites and errors
     */
    @Override
    public Result download(String s, int depth, List<String> list) {
        return generalDownload(s, depth, list, (str, substrs) ->
            substrs.stream().anyMatch(str::contains));
    }

    /**
     * close executors
     */
    @Override
    public void close() {
        downloaders.close();
        extractors.close();
    }

    /**
     *
     * @param s url
     * @param depth depth
     * @param hosts hosts, in which must be url
     * @return list of websites and errors
     */
    @Override
    public Result advancedDownload(String s, int depth, List<String> hosts) {
        return generalDownload(s, depth, hosts, (str, substrs) -> {
                String host = URLUtils.getHost(str);
                return substrs.contains(host);
        });
    }

    private Result generalDownload(String s, int depth, List<String> list, MyBiPredicate check) {
        TreeSet<String> set = new TreeSet<>(list);
        list = new ArrayList<>(set);
        Set<String> wasSet = ConcurrentHashMap.newKeySet();
        ConcurrentLinkedQueue<String> wasQueue = new ConcurrentLinkedQueue<>();
        ConcurrentHashMap<String, IOException> errors = new ConcurrentHashMap<>();
        ConcurrentLinkedQueue<String> currentLevel =  new ConcurrentLinkedQueue<>();
        currentLevel.add(s);
        try {
            if (!check.apply(s, list)) {
                return new Result(new ArrayList<>(), new HashMap<>());
            }
        } catch (MalformedURLException e) {
            errors.put(s, e);
        }
        for (int i = 1; i <= depth; i++) {
            ConcurrentLinkedQueue<String> newCurrentLevel = new ConcurrentLinkedQueue<>();
            Phaser phaser = new Phaser(1);
            for (String toDownload : currentLevel) {
                if (!wasSet.add(toDownload)) {
                    continue;
                }
                String host;
                try {
                    host = URLUtils.getHost(toDownload);
                } catch (MalformedURLException e) {
                    errors.put(toDownload, e);
                    continue;
                }
                int finalI = i;
                String finalHost = host;
                phaser.register();
                List<String> finalList = list;
                downloaders.submit(() -> {
                    try {
                        Semaphore semaphoreForThisHost = semaphores.computeIfAbsent(finalHost, _ -> new Semaphore(perHost));
                        try {
                            semaphoreForThisHost.acquire();
                            Document document;
                            try {
                                document = downloader.download(toDownload);
                            } catch (IOException e) {
                                errors.put(toDownload, e);
                                return;
                            }
                            Document finalDocument = document;
                            wasQueue.add(toDownload);
                            if (finalI < depth) {
                                phaser.register();
                                extractors.submit(() -> {
                                    List<String> result = null;
                                    try {
                                        result = finalDocument.extractLinks();
                                    } catch (IOException e) {
                                        errors.put(toDownload, e);
                                    } finally {
                                        if (result != null) {
                                            for (String inResult : result) {
                                                try {
                                                    if (!wasSet.contains(inResult) && check.apply(inResult, finalList)) {
                                                        newCurrentLevel.add(inResult);
                                                    }
                                                } catch (MalformedURLException e) {
                                                    errors.put(inResult, e);
                                                }
                                            }
                                        }
                                        phaser.arriveAndDeregister();
                                    }
                                });
                            }
                        } catch (InterruptedException e) {
                            Thread.currentThread().interrupt();
                        } finally {
                            semaphoreForThisHost.release();
                        }
                    } finally {
                        phaser.arriveAndDeregister();
                    }
                });
            }
            phaser.arriveAndAwaitAdvance();
            currentLevel = newCurrentLevel;
        }
        return new Result(new ArrayList<>(wasQueue), errors);
    }
}
