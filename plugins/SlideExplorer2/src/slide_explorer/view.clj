(ns slide-explorer.view
  (:import (javax.swing AbstractAction JComponent JFrame JPanel JLabel KeyStroke)
           (java.awt Color Graphics Graphics2D RenderingHints Window)
           (java.util UUID)
           (java.awt.event ComponentAdapter MouseAdapter WindowAdapter)
           (org.micromanager.utils GUIUpdater))
  (:use [org.micromanager.mm :only (edt)]
        [slide-explorer.image :only (crop overlay lut-object)]))

;; tile/pixels

(defn tile-to-pixels [[nx ny] [tile-width tile-height] tile-zoom]
  [(int (* (Math/pow 2.0 tile-zoom) nx tile-width))
   (int (* (Math/pow 2.0 tile-zoom) ny tile-height))])

(defn round-int
  "Round x to the nearest integer."
  [x]
  (Math/round (double x)))

(defn tiles-in-pixel-rectangle
  "Returns a list of tile indices found in a given pixel rectangle."
  [[l t b r] [tile-width tile-height]]
  (let [nl (round-int (/ l tile-width))
        nr (round-int (/ r tile-width))
        nt (round-int (/ t tile-height))
        nb (round-int (/ b tile-height))]
    (for [nx (range nl (inc nr))
          ny (range nt (inc nb))]
      [nx ny])))

;; gui utilities

(defn reference-viewer [reference key]
  (let [frame (JFrame. key)
        label (JLabel.)]
    (.add (.getContentPane frame) label)
    (add-watch reference key
               (fn [key reference old-state new-state]
                 (edt (.setText label (.toString new-state)))))
    (doto frame
      (.addWindowListener
        (proxy [WindowAdapter] []
          (windowClosing [e]
                         (remove-watch reference key))))
      .show))
  reference)

(defn bind-key
  "Maps an input-key on a swing component to an action,
  such that action-fn is executed when key is pressed."
  [component input-key action-fn global?]
  (let [im (.getInputMap component (if global?
                                     JComponent/WHEN_IN_FOCUSED_WINDOW
                                     JComponent/WHEN_FOCUSED))
        am (.getActionMap component)
        input-event (KeyStroke/getKeyStroke input-key)
        action
          (proxy [AbstractAction] []
            (actionPerformed [e]
                (action-fn)))
        uuid (.. UUID randomUUID toString)]
    (.put im input-event uuid)
    (.put am uuid action)))

(defn bind-window-key
  [window input-key action-fn]
  (bind-key (.getContentPane window) input-key action-fn true))

(defn- default-screen-device [] ; borrowed from see-saw
  (->
    (java.awt.GraphicsEnvironment/getLocalGraphicsEnvironment)
    .getDefaultScreenDevice))

(defn full-screen!
  "Make the given window/frame full-screen. Pass nil to return all windows
to normal size."
  ([^java.awt.GraphicsDevice device window]
    (if window
      (when (not= (.getFullScreenWindow device) window)
        (.dispose window)
        (.setUndecorated window true)
        (.setFullScreenWindow device window)
        (.show window))
      (when-let [window (.getFullScreenWindow device)]
        (.dispose window)
        (.setFullScreenWindow device nil)
        (.setUndecorated window false)
        (.show window)))
    window)
  ([window]
    (full-screen! (default-screen-device) window)))

(defn setup-fullscreen [window]
  (bind-window-key window "F" #(full-screen! window))
  (bind-window-key window "ESCAPE" #(full-screen! nil)))



(defn enable-anti-aliasing
  ([^Graphics g]
    (enable-anti-aliasing g true))
  ([^Graphics g on]
    (let [graphics2d (cast Graphics2D g)]
      (.setRenderingHint graphics2d
                         RenderingHints/KEY_ANTIALIASING
                         (if on
                           RenderingHints/VALUE_ANTIALIAS_ON
                           RenderingHints/VALUE_ANTIALIAS_OFF)))))
       
(defn paint-tiles [^Graphics2D g available-tiles zoom]
  (doseq [[{:keys [nx ny nz nt nc]} image] (get available-tiles zoom)]
    (let [[x y] (tile-to-pixels [nx ny] [512 512] zoom)]
      (when image
        (.drawImage g image
                    x y nil)))))

(defn paint-screen [graphics screen-state available-tiles]
  (let [original-transform (.getTransform graphics)]
    (doto graphics
      (.translate (+ (:x @screen-state) (/ (:width @screen-state) 2))
                  (+ (:y @screen-state) (/ (:height @screen-state) 2)))
      ;(.rotate @angle)
      enable-anti-aliasing
      (paint-tiles @available-tiles (:zoom @screen-state))
      (.setColor Color/YELLOW)
      (.fillOval -30 -30
                 60 60)
      (.setTransform original-transform)
      (.setColor Color/YELLOW)
      (.drawString (str @screen-state)
                   (int 0)
                   (int (- (:height @screen-state) 10))))))
  
(defn handle-drags [component position-atom]
  (let [drag-origin (atom nil)
        mouse-adapter
        (proxy [MouseAdapter] []
          (mousePressed [e]
                        (reset! drag-origin {:x (.getX e) :y (.getY e)}))
          (mouseReleased [e]
                         (reset! drag-origin nil))
          (mouseDragged [e]
                        (let [x (.getX e) y (.getY e)]
                          (swap! position-atom update-in [:x]
                                 + (- x (:x @drag-origin)))
                          (swap! position-atom update-in [:y]
                                 + (- y (:y @drag-origin)))
                          (reset! drag-origin {:x x :y y}))))]
    (doto component
      (.addMouseListener mouse-adapter)
      (.addMouseMotionListener mouse-adapter))
    position-atom))

(defn handle-wheel [component z-atom]
  (.addMouseWheelListener component
    (proxy [MouseAdapter] []
      (mouseWheelMoved [e]
                       (swap! z-atom update-in [:z]
                              + (.getWheelRotation e)))))
  z-atom)

(defn handle-resize [component size-atom]
  (let [update-size #(let [bounds (.getBounds component)]
                       (swap! size-atom merge
                              {:width (.getWidth bounds)
                               :height (.getHeight bounds)}))]
    (update-size)
    (.addComponentListener component
      (proxy [ComponentAdapter] []
        (componentResized [e]
                          (update-size)))))
  size-atom)

(defn handle-zoom [window zoom-atom]
  (bind-window-key window "ADD" #(swap! zoom-atom update-in [:zoom] inc))
  (bind-window-key window "SUBTRACT" #(swap! zoom-atom update-in [:zoom] dec)))

(defn display-follow [panel reference]
  (add-watch reference "display"
    (fn [_ _ _ _]
      (.repaint panel))))

(defn main-panel [screen-state available-tiles]
  (doto
    (let [updater (GUIUpdater.)]
      (proxy [JPanel] []
        (paintComponent [^Graphics graphics]
                        (proxy-super paintComponent graphics)
                        (paint-screen graphics screen-state available-tiles))
        (repaint []
                 (.post updater #(proxy-super repaint)))))
    (.setBackground Color/BLACK)))
    
(defn main-frame []
  (doto (JFrame. "Slide Explorer II")
    .show
    (.setBounds 10 10 500 500)))

(defn show [available-tiles]
  (let [screen-state (atom (sorted-map :x 0 :y 0 :z 0 :zoom 0))
        panel (main-panel screen-state available-tiles)
        frame (main-frame)]
    (def at available-tiles)
    (def ss screen-state)
    (def f frame)
    (def pnl panel)
    (.add (.getContentPane frame) panel)
    (setup-fullscreen frame)
    (handle-drags panel screen-state)
    (handle-wheel panel screen-state)
    (handle-resize panel screen-state)
    (handle-zoom frame screen-state)
    (display-follow panel screen-state)
    (display-follow panel available-tiles)
    frame))
