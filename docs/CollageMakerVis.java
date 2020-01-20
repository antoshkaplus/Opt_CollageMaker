import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.io.*;
import java.security.SecureRandom;
import java.util.*;
import java.util.List;

public class CollageMakerVis {
    public static final int ANSWER_LENGTH = 4 * TestCase.SOURCE_IMAGE_COUNT;

    public static String execCommand = null;
    public static long seed = 1;
    public static boolean vis = true;
    public static long quitTime = -1;
    public static String targetFolder = "300px";
    public static String sourceFolder = "100px";

    class Image {
        int H, W;
        int[][] pixels;

        void init(int H, int W) {
            this.H = H;
            this.W = W;
            this.pixels = new int[H][W];
        }

        public Image(int H, int W) {
            init(H, W);
        }

        public Image(String folder, String fileName) throws Exception {
            BufferedImage img;

            try {
                img = ImageIO.read(new File(folder, fileName));
            } catch (IOException e) {
                throw new Exception("Unable to read image from folder " + folder + " and file " + fileName + ".");
            }

            init(img.getHeight(), img.getWidth());

            int[] raster = img.getData().getPixels(0, 0, W, H, new int[H*W]);

            int pos = 0;
            for (int r=0; r < H; r++) {
                for (int c=0; c < W; c++) {
                    pixels[r][c] = raster[pos++];
                }
            }
        }

        void printSelf(List<Integer> lst) {
            lst.add(H);
            lst.add(W);
            for (int r=0; r < H; r++) {
                for (int c=0; c < W; c++) {
                    lst.add(pixels[r][c]);
                }
            }
        }

        int intersect(int a, int b, int c, int d) {
            int from = Math.max(a, c);
            int to = Math.min(b, d);
            return from < to ? to - from : 0;
        }

        Image scale(int newH, int newW) {
            List<Integer> origRList = new ArrayList<Integer>();
            List<Integer> newRList = new ArrayList<Integer>();
            List<Integer> intrRList = new ArrayList<Integer>();
            List<Integer> origCList = new ArrayList<Integer>();
            List<Integer> newCList = new ArrayList<Integer>();
            List<Integer> intrCList = new ArrayList<Integer>();

            for (int origR = 0; origR < H; origR++) {
                int r1 = origR * newH, r2 = r1 + newH;
                for (int newR = 0; newR < newH; newR++) {
                    int r3 = newR * H, r4 = r3 + H;
                    int intr = intersect(r1, r2, r3, r4);
                    if (intr > 0) {
                        origRList.add(origR);
                        newRList.add(newR);
                        intrRList.add(intr);
                    }
                }
            }

            for (int origC = 0; origC < W; origC++) {
                int c1 = origC * newW, c2 = c1 + newW;
                for (int newC = 0; newC < newW; newC++) {
                    int c3 = newC * W, c4 = c3 + W;
                    int intr = intersect(c1, c2, c3, c4);
                    if (intr > 0) {
                        origCList.add(origC);
                        newCList.add(newC);
                        intrCList.add(intr);
                    }
                }
            }

            Image res = new Image(newH, newW);

            for (int i = 0; i < origRList.size(); i++) {
                int origR = origRList.get(i);
                int newR = newRList.get(i);
                int intrR = intrRList.get(i);

                for (int j = 0; j < origCList.size(); j++) {
                    int origC = origCList.get(j);
                    int newC = newCList.get(j);
                    int intrC = intrCList.get(j);

                    res.pixels[newR][newC] += intrR * intrC * pixels[origR][origC];
                }
            }

            for (int r = 0; r < newH; r++) {
                for (int c = 0; c < newW; c++) {
                    res.pixels[r][c] = (2 * res.pixels[r][c] + H * W) / (2 * H * W);
                }
            }

            return res;
        }
    }

    class TestCase {
        public static final int TOTAL_IMAGE_COUNT = 1000;
        public static final int SOURCE_IMAGE_COUNT = 200;

        SecureRandom rnd = null;
        Set<Integer> IDs = new HashSet<Integer>();

        Image target;
        Image[] source;

        private int getFreeID() {
            while (true) {
                int id = rnd.nextInt(TOTAL_IMAGE_COUNT);
                if (!IDs.contains(id)) {
                    IDs.add(id);
                    return id;
                }
            }
        }

        public TestCase(long seed) throws Exception {
            rnd = null;

            try {
                rnd = SecureRandom.getInstance("SHA1PRNG");
            } catch (Exception e) {
                System.err.println("ERROR: unable to generate test case.");
                System.exit(1);
            }

            rnd.setSeed(seed);

            target = new Image(targetFolder, (seed % TOTAL_IMAGE_COUNT) + ".png");
            IDs.add((int)(seed % TOTAL_IMAGE_COUNT));

            source = new Image[SOURCE_IMAGE_COUNT];
            for (int i=0; i < SOURCE_IMAGE_COUNT; i++) {
                source[i] = new Image(sourceFolder, getFreeID() + ".png");
            }
        }
    }

    public int[] runSolution(TestCase tc, String execCmd) throws Exception {
        Process solution = null;

        try {
            try {
                solution = Runtime.getRuntime().exec(execCmd);
            } catch (Exception e) {
                throw new Exception("Unable to execute your solution using the provided command: " + execCmd + ".");
            }

            BufferedReader reader = new BufferedReader(new InputStreamReader(solution.getInputStream()));
            PrintWriter writer = new PrintWriter(solution.getOutputStream());
            new ErrorStreamRedirector(solution.getErrorStream()).start();

            List<Integer> data = new ArrayList<Integer>();

            tc.target.printSelf(data);
            for (Image img : tc.source) {
                img.printSelf(data);
            }
            writer.println(data.size());
            for (int elm : data) {
                writer.println(elm);
            }
            writer.flush();

            int[] ans = new int[ANSWER_LENGTH];
            for (int i=0; i < ans.length; i++) {
                try {
                    ans[i] = Integer.parseInt(reader.readLine());
                } catch (Exception e) {
                    throw new Exception("Unable to read element " + i + " (0-based) of your solution's return value.");
                }
            }

            return ans;
        } finally {
            if (solution != null) {
                try {
                    solution.destroy();
                } catch (Exception e) {
                    // do nothing
                }
            }
        }
    }

    class Drawer extends JFrame {
        class DrawerWindowListener extends WindowAdapter {
            public void windowClosing(WindowEvent event) {
                System.exit(0);
            }
        }

        class DrawerPanel extends JPanel {
            public void paint(Graphics g) {
                g.drawImage(target, 66, 40, null);
                g.drawImage(collage, 133 + target.getWidth(), 40, null);
                g.drawImage(bndCollage, 66, 80 + target.getHeight(), null);
                g.drawImage(diff, 133 + target.getWidth(), 80 + target.getHeight(), null);
            }
        }

        DrawerPanel panel;
        int width, height;

        BufferedImage target, collage;
        BufferedImage bndCollage, diff;

        final int EXTRA_HEIGHT = 150;
        final int EXTRA_WIDTH = 200;

        public Drawer(Image target, Image collage, int[] ans) {
            super();

            this.target = new BufferedImage(target.W, target.H, BufferedImage.TYPE_INT_RGB);
            this.collage = new BufferedImage(target.W, target.H, BufferedImage.TYPE_INT_RGB);
            this.bndCollage = new BufferedImage(target.W, target.H, BufferedImage.TYPE_INT_RGB);
            this.diff = new BufferedImage(target.W, target.H, BufferedImage.TYPE_INT_RGB);

            for (int r = 0; r < target.H; r++) {
                for (int c = 0; c < target.W; c++) {
                    int vt = target.pixels[r][c];
                    this.target.setRGB(c, r, new Color(vt, vt, vt).getRGB());

                    int vc = collage.pixels[r][c];
                    this.collage.setRGB(c, r, new Color(vc, vc, vc).getRGB());
                    this.bndCollage.setRGB(c, r, new Color(vc, vc, vc).getRGB());

                    if (vt <= vc) {
                        int d = vc - vt;
                        this.diff.setRGB(c, r, new Color(255, 255 - d, 255 - d).getRGB());
                    } else {
                        int d = vt - vc;
                        this.diff.setRGB(c, r, new Color(255 - d, 255 - d, 255).getRGB());
                    }
                }
            }

            for (int i=0; i < ans.length / 4; i++) {
                int fromR = ans[4 * i], fromC = ans[4 * i + 1];
                int toR = ans[4 * i + 2], toC = ans[4 * i + 3];

                if (fromR == -1) {
                    continue;
                }

                for (int r = fromR; r <= toR; r++) {
                    for (int c = fromC; c <= toC; c++) {
                        if (r == fromR || r == toR || c == fromC || c == toC) {
                            this.bndCollage.setRGB(c, r, new Color(255, 0, 0).getRGB());
                        }
                    }
                }
            }

            panel = new DrawerPanel();
            getContentPane().add(panel);

            addWindowListener(new DrawerWindowListener());

            width = EXTRA_WIDTH + 2 * target.W;
            height = EXTRA_HEIGHT + 2 * target.H;

            setSize(width, height);
            setTitle("TCO'14 Marathon Round 3");

            setResizable(false);
            setVisible(true);
        }
    }

    double runTest() throws Exception {
        TestCase tc = new TestCase(seed);

        int[] ans = runSolution(tc, execCommand);

        Image collage = new Image(tc.target.H, tc.target.W);
        for (int i=0; i < tc.target.H; i++) {
            Arrays.fill(collage.pixels[i], -1);
        }

        for (int i = 0; i < tc.SOURCE_IMAGE_COUNT; i++) {
            int fromR = ans[4 * i], fromC = ans[4 * i + 1];
            int toR = ans[4 * i + 2], toC = ans[4 * i + 3];

            if (fromR == -1 && fromC == -1 && toR == -1 && toC == -1) {
                continue;
            }

            if (fromR < 0 || fromR >= tc.target.H) {
                System.err.println("ERROR: top row for " + i + "-th source image (0-based) must be from 0" +
                        " to " + (tc.target.H-1) + ", inclusive. In your return value it is equal to " + fromR + ".");
                return -1;
            }

            if (toR < 0 || toR >= tc.target.H) {
                System.err.println("ERROR: bottom row for " + i + "-th source image (0-based) must be from 0" +
                        " to " + (tc.target.H-1) + ", inclusive. In your return value it is equal to " + toR + ".");
                return -1;
            }

            if (fromC < 0 || fromC >= tc.target.W) {
                System.err.println("ERROR: left column for " + i + "-th source image (0-based) must be from 0" +
                        " to " + (tc.target.W-1) + ", inclusive. In your return value it is equal to " + fromC + ".");
                return -1;
            }

            if (toC < 0 || toC >= tc.target.W) {
                System.err.println("ERROR: right column for " + i + "-th source image (0-based) must be from 0" +
                        " to " + (tc.target.W-1) + ", inclusive. In your return value it is equal to " + toC + ".");
                return -1;
            }

            if (fromR > toR) {
                System.err.println("ERROR: top row for " + i + "-th source image (0-based) can't be larger than" +
                        " bottom row. In your return value we have " + fromR + " > " + toR + ".");
                return -1;
            }

            if (fromC > toC) {
                System.err.println("ERROR: left column for " + i + "-th source image (0-based) can't be larger than" +
                        " right column. In your return value we have " + fromC + " > " + toC + ".");
                return -1;
            }

            int newH = toR - fromR + 1;
            if (newH > tc.source[i].H) {
                System.err.println("ERROR: scaled height for " + i + "-th source image (0-based) can't be larger than" +
                        " its original height. In your return value we have " + newH + " > " + tc.source[i].H + ".");
                return -1;
            }

            int newW = toC - fromC + 1;
            if (newW > tc.source[i].W) {
                System.err.println("ERROR: scaled width for " + i + "-th source image (0-based) can't be larger than" +
                        " its original width. In your return value we have " + newW + " > " + tc.source[i].W + ".");
                return -1;
            }

            Image scaled = tc.source[i].scale(newH, newW);

            for (int r = fromR; r <= toR; r++) {
                for (int c = fromC; c <= toC; c++) {
                    if (collage.pixels[r][c] != -1) {
                        System.err.println("ERROR: pixel at row " + r + ", column " + c + " (0-based) in target" +
                                " image is covered by at least 2 source images.");
                        return -1;
                    }
                    collage.pixels[r][c] = scaled.pixels[r - fromR][c - fromC];
                }
            }
        }

        double score = 0.0;

        for (int r = 0; r < collage.H; r++) {
            for (int c = 0; c < collage.W; c++) {
                if (collage.pixels[r][c] == -1) {
                    System.err.println("ERROR: pixel at row " + r + ", column " + c + " (0-based) in target image" +
                            " is not covered by any source images.");
                    return -1;
                }
                int diff = tc.target.pixels[r][c] - collage.pixels[r][c];
                score += diff * diff;
            }
        }

        if (vis) {
            new Drawer(tc.target, collage, ans);
        }

        return Math.sqrt(score / (tc.target.H * tc.target.W));
    }

    public static void main(String[] args) throws Exception {
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-exec")) {
                execCommand = args[++i];
            } else if (args[i].equals("-seed")) {
                seed = Long.parseLong(args[++i]);
            } else if (args[i].equals("-novis")) {
                vis = false;
            } else if (args[i].equals("-quit")) {
                quitTime = Long.parseLong(args[++i]);
            } else if (args[i].equals("-target")) {
                targetFolder = args[++i];
            } else if (args[i].equals("-source")) {
                sourceFolder = args[++i];
            } else {
                System.out.println("WARNING: unknown argument " + args[i] + ".");
            }
        }

        if (execCommand == null) {
            System.err.println("ERROR: You did not provide the command to execute your solution." +
                    " Please use -exec <command> for this.");
            System.exit(1);
        }

        CollageMakerVis vis = new CollageMakerVis();
        try {
            double score = vis.runTest();
            System.out.println("Score = " + score);
            if (quitTime >= 0) {
                Thread.sleep(quitTime);
                System.exit(0);
            }
        } catch (Exception e) {
            System.err.println("ERROR: Unexpected error while running your test case.");
            e.printStackTrace();
        }
    }
}

class ErrorStreamRedirector extends Thread {
    public BufferedReader reader;

    public ErrorStreamRedirector(InputStream is) {
        reader = new BufferedReader(new InputStreamReader(is));
    }

    public void run() {
        while (true) {
            String s;
            try {
                s = reader.readLine();
            } catch (Exception e) {
                // e.printStackTrace();
                return;
            }
            if (s == null) {
                break;
            }
            System.out.println(s);
        }
    }
}
