package edu.neu.explore;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

// Spring Boot test app for the Sprint 1 exploration.
//   mvn -q spring-boot:run        (listens on 8003)
@SpringBootApplication
@RestController
public class ExploreApplication {

    public static void main(String[] args) {
        SpringApplication.run(ExploreApplication.class, args);
    }

    @GetMapping("/hello")
    public String hello() {
        return "hello from spring boot (" + Thread.currentThread().getName() + ")\n";
    }

    // CPU-bound: busy-loop on the Tomcat worker thread
    @GetMapping("/cpu")
    public String cpu(@RequestParam(defaultValue = "20") int ms) {
        long end = System.nanoTime() + ms * 1_000_000L;
        long x = 0;
        while (System.nanoTime() < end) x++;
        return "cpu " + ms + "ms\n";
    }

    // I/O-bound: the Tomcat worker thread blocks while it sleeps
    @GetMapping("/io")
    public String io(@RequestParam(defaultValue = "20") int ms) throws InterruptedException {
        Thread.sleep(ms);
        return "io " + ms + "ms\n";
    }
}
