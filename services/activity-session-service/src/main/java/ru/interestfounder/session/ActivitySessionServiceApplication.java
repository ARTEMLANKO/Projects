package ru.interestfounder.session;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.scheduling.annotation.EnableScheduling;

@SpringBootApplication
public class ActivitySessionServiceApplication {

    public static void main(String[] args) {
        SpringApplication.run(ActivitySessionServiceApplication.class, args);
    }
}
